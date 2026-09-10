'use strict';
const fs=require('node:fs/promises');
const path=require('node:path');
const os=require('node:os');
const {spawn}=require('node:child_process');
const {TutorError}=require('./tutor-core.cjs');

// Secret goes through stdin, never shell arguments, command interpolation or logs.
function protect(value,decrypt=false) {
  if(process.platform!=='win32')return Promise.reject(Error('持久化 Key 仅支持 Windows DPAPI；其他系统请设置 BLASTER_TUTOR_API_KEY 环境变量'));
  const command='$ErrorActionPreference="Stop"; '+(decrypt
    ? '$s=[Console]::In.ReadToEnd(); $v=ConvertTo-SecureString $s; $p=[Runtime.InteropServices.Marshal]::SecureStringToBSTR($v); try {[Console]::Out.Write([Runtime.InteropServices.Marshal]::PtrToStringBSTR($p))} finally {[Runtime.InteropServices.Marshal]::ZeroFreeBSTR($p)}'
    : '$s=[Console]::In.ReadToEnd(); $v=ConvertTo-SecureString $s -AsPlainText -Force; [Console]::Out.Write((ConvertFrom-SecureString $v))');
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
function createConfigStore({directory=path.join(process.env.LOCALAPPDATA||path.join(os.homedir(),'.config'),'BlasterLab','assistant'),io=fs}={}) {
  const file=path.join(directory,'config.json');
  const backup=file+'.previous';
  const defaults={baseUrl:'https://api.openai.com/v1',model:'',apiMode:'chat',apiKey:''};
  return {
    async read(){
      let config={...defaults};
      try{
        let stored;
        try{stored=JSON.parse(await io.readFile(file,'utf8'));}
        catch(primaryError){try{stored=JSON.parse(await io.readFile(backup,'utf8'));}catch{throw primaryError;}}
        config={baseUrl:stored.baseUrl,model:stored.model,apiMode:stored.apiMode,apiKey:stored.protectedKey?await protect(stored.protectedKey,true):''};
      }
      catch(error){if(error.code!=='ENOENT')throw Error('无法读取本机助教设置，请检查本用户的配置文件');}
      if(process.env.BLASTER_TUTOR_API_KEY)config.apiKey=process.env.BLASTER_TUTOR_API_KEY;
      if(process.env.BLASTER_TUTOR_BASE_URL)config.baseUrl=process.env.BLASTER_TUTOR_BASE_URL;
      if(process.env.BLASTER_TUTOR_MODEL)config.model=process.env.BLASTER_TUTOR_MODEL;
      return config;
    },
    async write(config){
      if(process.env.BLASTER_TUTOR_API_KEY||process.env.BLASTER_TUTOR_BASE_URL||process.env.BLASTER_TUTOR_MODEL)throw new TutorError('当前使用环境变量配置，请在启动环境中修改并重启',409);
      let protectedKey;
      try{protectedKey=config.apiKey?await protect(config.apiKey):'';}
      catch{throw new TutorError('Windows 用户密钥加密失败，请以当前登录用户重新启动学习助教后重试；尚未调用 API。',500);}
      const temp=file+'.tmp';
      try{
        await io.mkdir(directory,{recursive:true,mode:0o700});
        await io.writeFile(temp,JSON.stringify({baseUrl:config.baseUrl,model:config.model,apiMode:config.apiMode,protectedKey},null,2),{mode:0o600});
        try{await io.rename(temp,file);}
        catch(error){
          if(error.code!=='EXDEV')throw error;
          // Windows EFS can reject rename even within one directory. Only the
          // DPAPI ciphertext is copied; keep a recoverable old config until done.
          let hadPrevious=false;
          let previousSource=file;
          try{
            JSON.parse(await io.readFile(file,'utf8'));hadPrevious=true;
          }catch(previousError){
            try{JSON.parse(await io.readFile(backup,'utf8'));hadPrevious=true;previousSource=backup;}
            catch{if(previousError.code!=='ENOENT')throw previousError;}
          }
          if(hadPrevious&&previousSource===file)await io.copyFile(file,backup);
          try{await io.copyFile(temp,file);}
          catch(copyError){
            if(hadPrevious)await io.copyFile(backup,file).catch(()=>{});
            else await io.unlink(file).catch(()=>{});
            throw copyError;
          }
        }
        await io.unlink(backup).catch(error=>{if(error.code!=='ENOENT')throw error;});
      }catch{throw new TutorError('本机配置文件保存失败，请检查 Windows 用户目录的写入权限或磁盘空间；尚未调用 API，学习记录不受影响。',500);}
      finally{await io.unlink(temp).catch(()=>{});}
    }
  };
}
module.exports={createConfigStore,protect};
