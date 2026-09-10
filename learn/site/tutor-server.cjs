'use strict';
const http=require('node:http');
const fs=require('node:fs');
const path=require('node:path');
const vm=require('node:vm');
const crypto=require('node:crypto');
const {TutorError,validateSettings,buildMessages,parseEvaluation,requestCompletion}=require('./tutor-core.cjs');
const {createConfigStore}=require('./tutor-config.cjs');
const ROOT=path.resolve(__dirname,'..');
const STATIC=new Map([['/','index.html'],['/index.html','index.html'],...['style.css','content.js','progress.js','app.js','tutor.js','tutor.css','assets/remiel.png'].map(p=>['/site/'+p,'site/'+p])]);
function loadCourse(){const scope={window:{}};vm.runInNewContext(fs.readFileSync(path.join(__dirname,'content.js'),'utf8'),scope,{timeout:1500});return scope.window.COURSE;}
function publicSettings(config){return {baseUrl:config.baseUrl,model:config.model,apiMode:config.apiMode,hasKey:!!config.apiKey};}
async function readBody(req){
  if(!(req.headers['content-type']||'').startsWith('application/json'))throw new TutorError('只接受 JSON 请求',415);
  const chunks=[];let size=0;for await(const chunk of req){size+=chunk.length;if(size>128000)throw new TutorError('请求内容过长',413);chunks.push(chunk);}
  try{return JSON.parse(Buffer.concat(chunks).toString('utf8'));}catch{throw new TutorError('JSON 请求无效');}
}
function createTutorServer({store=createConfigStore(),course=loadCourse(),callModel=requestCompletion}={}) {
  const token=crypto.randomBytes(32).toString('hex');let active=false;
  const server=http.createServer(async(req,res)=>{
    const port=server.address().port;const origin=`http://127.0.0.1:${port}`;
    const send=(status,data)=>{if(!res.destroyed){res.writeHead(status,{'Content-Type':'application/json; charset=utf-8','Cache-Control':'no-store','X-Content-Type-Options':'nosniff'});res.end(JSON.stringify(data));}};
    if(req.headers.host!==`127.0.0.1:${port}` || (req.headers.origin&&req.headers.origin!==origin)){send(403,{error:'只允许本机同源页面访问'});return;}
    let acquired=false;
    try{
      const pathname=new URL(req.url,origin).pathname;
      if(req.method==='GET'&&pathname==='/api/session'){
        let config;try{config=await store.read();}catch{config={baseUrl:'https://api.openai.com/v1',model:'',apiMode:'chat',apiKey:''};}
        send(200,{app:'BlasterLabTutor',token,configured:!!(config.model&&config.apiKey),settings:publicSettings(config)});return;
      }
      if(req.method==='GET'&&STATIC.has(pathname)){
        const file=path.join(ROOT,STATIC.get(pathname));if(!fs.existsSync(file)){send(404,{error:'文件不存在'});return;}
        const type=file.endsWith('.css')?'text/css':file.endsWith('.js')?'text/javascript':file.endsWith('.png')?'image/png':'text/html';
        res.writeHead(200,{'Content-Type':type+'; charset=utf-8','X-Content-Type-Options':'nosniff','Cache-Control':'no-cache','Content-Security-Policy':"default-src 'self'; connect-src 'self'; img-src 'self' data:; style-src 'self' 'unsafe-inline'; script-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'",'Referrer-Policy':'no-referrer'});fs.createReadStream(file).pipe(res);return;
      }
      if(req.method!=='POST'||!['/api/settings','/api/message'].includes(pathname)){send(404,{error:'未找到'});return;}
      if(req.headers.origin!==origin||req.headers['x-blaster-token']!==token){send(403,{error:'页面会话无效，请刷新本机学习页面'});return;}
      if(active){send(409,{error:'助教正在处理请求，请稍后再试'});return;}
      active=true;acquired=true;
      const body=await readBody(req);
      if(pathname==='/api/settings'){
        const settings=validateSettings(body);let previous;
        try{previous=await store.read();}catch{previous={baseUrl:'',apiKey:''};}
        if(typeof body.apiKey!=='string'||body.apiKey.length>4096||/[\r\n]/.test(body.apiKey))throw new TutorError('Key 格式不正确');
        const newKey=body.apiKey.trim();
        if(settings.baseUrl!==previous.baseUrl&&!newKey&&!body.clearKey)throw new TutorError('更换 API 地址时请重新填写 Key，避免将旧密钥发往其他服务');
        const next={...settings,apiKey:body.clearKey?'':newKey||previous.apiKey};
        await store.write(next);
        send(200,{settings:publicSettings(next),configured:!!next.apiKey});return;
      }
      const messages=buildMessages(course,body);const config=await store.read();
      if(!config.apiKey||!config.model)throw new TutorError('请先在助教设置中填写 API 地址、模型和 Key',409);
      validateSettings(config);
      const abort=new AbortController();res.on('close',()=>{if(!res.writableEnded)abort.abort();});
      try{
        const reply=await callModel(config,messages,{signal:abort.signal});
        const safe=String(reply).split(config.apiKey).join('[已隐藏密钥]');
        const evaluation=body.mode==='evaluate'?parseEvaluation(safe):undefined;
        send(200,{reply:evaluation?evaluation.reply:safe,evaluation,model:config.model,lessonId:body.lessonId,createdAt:new Date().toISOString()});
      }finally{abort.abort();}
    }catch(error){send(error instanceof TutorError?error.status:502,{error:error instanceof TutorError?error.message:'本机设置或 API 暂不可用，请检查连接与配置后重试；原学习记录保留'});}
    finally{if(acquired)active=false;}
  });
  server.requestTimeout=100000;server.headersTimeout=10000;
  return server;
}
if(require.main===module){
  const port=Number(process.env.BLASTER_TUTOR_PORT||38761);
  if(!Number.isInteger(port)||port<1024||port>65535)throw Error('无效的本机端口');
  const server=createTutorServer();server.on('error',()=>{console.error('助教服务无法启动：请检查端口是否占用。');process.exitCode=1;});
  server.listen(port,'127.0.0.1',()=>console.log(`Blaster Lab tutor: http://127.0.0.1:${port}`));
}
module.exports={createTutorServer,loadCourse};
