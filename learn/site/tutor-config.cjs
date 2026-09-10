'use strict';
const fs=require('node:fs/promises');
const path=require('node:path');
const os=require('node:os');
const {spawn}=require('node:child_process');

// Secret goes through stdin, never shell arguments, command interpolation or logs.
function protect(value,decrypt=false) {
  if(process.platform!=='win32')return Promise.reject(Error('持久化 Key 仅支持 Windows DPAPI；其他系统请设置 BLASTER_TUTOR_API_KEY 环境变量'));
  const command=decrypt
    ? '$s=[Console]::In.ReadToEnd(); $v=ConvertTo-SecureString $s; $p=[Runtime.InteropServices.Marshal]::SecureStringToBSTR($v); try {[Console]::Out.Write([Runtime.InteropServices.Marshal]::PtrToStringBSTR($p))} finally {[Runtime.InteropServices.Marshal]::ZeroFreeBSTR($p)}'
    : '$s=[Console]::In.ReadToEnd(); $v=ConvertTo-SecureString $s -AsPlainText -Force; [Console]::Out.Write((ConvertFrom-SecureString $v))';
  return new Promise((resolve,reject)=>{
    // PowerShell 7 can pass an incompatible module search path to Windows PS 5.
    const env=Object.fromEntries(Object.entries(process.env).filter(([name])=>name.toLowerCase()!=='psmodulepath'));
    env.PSModulePath=path.join(process.env.SystemRoot||'C:\\Windows','System32','WindowsPowerShell','v1.0','Modules');
    const child=spawn('powershell.exe',['-NoProfile','-NonInteractive','-EncodedCommand',Buffer.from(command,'utf16le').toString('base64')],{windowsHide:true,env,stdio:['pipe','pipe','pipe']});
    let output='';const timer=setTimeout(()=>{child.kill();reject(Error('Windows 密钥保护操作超时'));},15000);
    child.stdout.on('data',chunk=>output+=chunk);child.stderr.resume();
    child.on('error',()=>{clearTimeout(timer);reject(Error('无法使用 Windows 密钥保护'));});
    child.on('close',code=>{clearTimeout(timer);code===0?resolve(output):reject(Error('无法解密本用户的 Key，请在设置中重新填写'));});child.stdin.end(value);
  });
}
function createConfigStore({directory=path.join(process.env.LOCALAPPDATA||path.join(os.homedir(),'.config'),'BlasterLab','assistant')}={}) {
  const file=path.join(directory,'config.json');
  const defaults={baseUrl:'https://api.openai.com/v1',model:'',apiMode:'chat',apiKey:''};
  return {
    async read(){
      let config={...defaults};
      try{const stored=JSON.parse(await fs.readFile(file,'utf8'));config={baseUrl:stored.baseUrl,model:stored.model,apiMode:stored.apiMode,apiKey:stored.protectedKey?await protect(stored.protectedKey,true):''};}
      catch(error){if(error.code!=='ENOENT')throw Error('无法读取本机助教设置，请检查本用户的配置文件');}
      if(process.env.BLASTER_TUTOR_API_KEY)config.apiKey=process.env.BLASTER_TUTOR_API_KEY;
      if(process.env.BLASTER_TUTOR_BASE_URL)config.baseUrl=process.env.BLASTER_TUTOR_BASE_URL;
      if(process.env.BLASTER_TUTOR_MODEL)config.model=process.env.BLASTER_TUTOR_MODEL;
      return config;
    },
    async write(config){
      if(process.env.BLASTER_TUTOR_API_KEY||process.env.BLASTER_TUTOR_BASE_URL||process.env.BLASTER_TUTOR_MODEL)throw Error('当前使用环境变量配置，请在启动环境中修改并重启');
      const protectedKey=config.apiKey?await protect(config.apiKey):'';
      await fs.mkdir(directory,{recursive:true,mode:0o700});
      const temp=file+'.tmp';
      await fs.writeFile(temp,JSON.stringify({baseUrl:config.baseUrl,model:config.model,apiMode:config.apiMode,protectedKey},null,2),{mode:0o600});
      await fs.rename(temp,file);
    }
  };
}
module.exports={createConfigStore,protect};
