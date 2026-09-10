const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs/promises');
const path=require('node:path');
const {createConfigStore,protect}=require('./tutor-config.cjs');
test('encrypted directory EXDEV saves and updates without losing previous configuration',async()=>{
  const folder=path.join(__dirname,'test-results','efs-fixture');await fs.mkdir(folder,{recursive:true});
  const io={...fs,rename:async()=>{throw Object.assign(Error('fixture'),{code:'EXDEV'});}};
  const store=createConfigStore({directory:folder,io});
  await store.write({baseUrl:'https://example.com',model:'first',apiMode:'responses',apiKey:''});
  await store.write({baseUrl:'https://example.com',model:'second',apiMode:'responses',apiKey:''});
  assert.equal((await store.read()).model,'second');
  await fs.copyFile(path.join(folder,'config.json'),path.join(folder,'config.json.previous'));
  await fs.writeFile(path.join(folder,'config.json'),'interrupted-copy');
  assert.equal((await store.read()).model,'second');
  await store.write({baseUrl:'https://example.com',model:'second',apiMode:'responses',apiKey:''});
  await assert.rejects(()=>fs.access(path.join(folder,'config.json.previous')));
  const failing={...io,copyFile:async(from,to,...args)=>{
    if(from.endsWith('.tmp')){await fs.writeFile(to,'partial');throw Object.assign(Error('fixture'),{code:'EACCES'});}
    return fs.copyFile(from,to,...args);
  }};
  await assert.rejects(()=>createConfigStore({directory:folder,io:failing}).write({model:'lost',apiKey:''}));
  assert.equal((await store.read()).model,'second');
});
test('Windows DPAPI protects persisted test credential and can restore/update/remove it',{skip:process.platform!=='win32'},async()=>{
  const folder=process.env.BLASTER_TUTOR_TEST_EFS==='1'?path.join(process.env.LOCALAPPDATA,'BlasterLab','assistant','diagnostic-fixture'):path.join(__dirname,'test-results','config-fixture');await fs.mkdir(folder,{recursive:true});
  const names=['BLASTER_TUTOR_API_KEY','BLASTER_TUTOR_BASE_URL','BLASTER_TUTOR_MODEL'];const previous=Object.fromEntries(names.map(n=>[n,process.env[n]]));
  try{
    for(const n of names)delete process.env[n];
    const store=createConfigStore({directory:folder}),secret=['fixture','protected','credential'].join('-');
    await store.write({baseUrl:'https://example.com/v1',model:'fixture-model',apiMode:'chat',apiKey:secret});
    const raw=await fs.readFile(path.join(folder,'config.json'),'utf8');assert.ok(!raw.includes(secret));assert.ok(JSON.parse(raw).protectedKey.length>20);
    assert.equal((await store.read()).apiKey,secret);
    process.env.BLASTER_TUTOR_API_KEY='environment-fixture';assert.equal((await store.read()).apiKey,'environment-fixture');await assert.rejects(()=>store.write({apiKey:'x'}));delete process.env.BLASTER_TUTOR_API_KEY;
    await store.write({baseUrl:'https://example.com/v1',model:'fixture-model',apiMode:'chat',apiKey:''});assert.equal((await store.read()).apiKey,'');
    await assert.rejects(()=>protect('invalid-cipher',true));
    assert.equal((await createConfigStore({directory:path.join(folder,'missing')}).read()).apiKey,'');
  }finally{for(const n of names){if(previous[n]===undefined)delete process.env[n];else process.env[n]=previous[n];}}
});
