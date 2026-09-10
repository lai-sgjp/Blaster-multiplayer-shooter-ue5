const {chromium}=require('playwright');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const {createTutorServer}=require('./tutor-server.cjs');
const fakeKey=['test','not-a-real-secret','temporary'].join('-');
const grade={reply:'共犯，这次抓住了服务器裁决的重点。<script>window.pwned=true</script>',score:82,strengths:['区分了数据与 UI'],gaps:['补充 ServerFire 的 Owner 校验'],nextStep:'追踪服务器扣弹的位置。',suggestion:{concept:true,source:true,interview:false,review:true}};
(async()=>{
  let cfg={baseUrl:'https://api.example.test/v1',model:'',apiMode:'chat',apiKey:''},lastMessages;
  const store={read:async()=>({...cfg}),write:async c=>{cfg={...c};}};
  const server=createTutorServer({store,callModel:async(c,m)=>{lastMessages=m;return JSON.parse(m.at(-1).content).task==='evaluate'?JSON.stringify(grade):'Owner 是连接归属，Authority 是权威裁决。请看 ServerFire。';}});
  await new Promise(r=>server.listen(0,'127.0.0.1',r));const base=`http://127.0.0.1:${server.address().port}`;
  const browser=await chromium.launch({headless:true,channel:'msedge'});
  try{
    const context=await browser.newContext({viewport:{width:1440,height:1000},acceptDownloads:true});const page=await context.newPage();const errors=[];page.on('pageerror',e=>errors.push(e.message));
    await page.goto(base+'/#lesson=L03');await page.locator('#speech').fill('服务器修改权威状态，客户端只显示 UI。');await page.locator('#tutor-launch').click();
    await page.locator('#tutor-settings:not([hidden])').waitFor();await page.locator('#tutor-base').fill('https://api.example.test/v1');await page.locator('#tutor-model').fill('test-model');await page.locator('#tutor-key').fill(fakeKey);await page.locator('#tutor-settings [type=submit]').click();await page.waitForFunction(()=>document.querySelector('#tutor-settings').hidden);
    assert.equal(await page.locator('#tutor-key').inputValue(),'');assert.ok(!(await page.evaluate(()=>JSON.stringify(localStorage))).includes(fakeKey));
    await page.locator('#tutor-question').fill('能讲清 Owner 与 Authority 吗？');await page.locator('#tutor-send').click();await page.locator('.tutor-message.assistant').waitFor();assert.match(await page.locator('.tutor-message.assistant').textContent(),/ServerFire/);
    await page.locator('#tutor-evaluate').click();await page.locator('.tutor-assessment').waitFor();assert.match(await page.locator('.tutor-score').textContent(),/82/);assert.equal(await page.locator('[data-check="concept"]').isChecked(),false);assert.equal(await page.evaluate(()=>window.pwned),undefined);
    const submitted=JSON.parse(lastMessages.at(-1).content);assert.equal(submitted.lesson.id,'L03');assert.match(submitted.learnerAnswer,/服务器修改/);assert.ok(!JSON.stringify(lastMessages).includes(fakeKey));
    await page.locator('[data-adopt]').click();assert.equal(await page.locator('[data-check="concept"]').isChecked(),true);assert.equal(await page.locator('[data-check="source"]').isChecked(),true);assert.equal(await page.locator('[data-check="evidence"]').isChecked(),false);
    const out=path.join(__dirname,'test-results');fs.mkdirSync(out,{recursive:true});await page.screenshot({path:path.join(out,'tutor-desktop.png'),fullPage:false});
    await page.reload();await page.locator('#tutor-launch').click();await page.locator('.tutor-assessment').waitFor();assert.match(await page.locator('[data-adopt]').textContent(),/已采纳/);
    await page.locator('#tutor-close').click();const downloadEvent=page.waitForEvent('download');await page.locator('#export').click();const backup=path.join(out,'tutor-backup.json');await (await downloadEvent).saveAs(backup);const raw=fs.readFileSync(backup,'utf8');assert.ok(!raw.includes(fakeKey));assert.equal(JSON.parse(raw).lessons.L03.assessments.length,1);
    await page.evaluate(()=>localStorage.clear());await page.reload();await page.locator('#import').setInputFiles(backup);await page.waitForFunction(()=>document.querySelector('#notice').textContent.includes('已恢复'));await page.locator('#tutor-launch').click();await page.locator('.tutor-assessment').waitFor();
    await page.setViewportSize({width:390,height:844});assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);await page.screenshot({path:path.join(out,'tutor-mobile.png'),fullPage:false});
    assert.deepEqual(errors,[]);console.log('PASS: assistant settings, no browser key, contextual chat/evaluation, explicit progress adoption, no fabricated evidence, stored history, backup/restore, XSS-as-text, desktop/mobile');
  }finally{await browser.close();await new Promise(r=>server.close(r));}
})().catch(e=>{console.error(e);process.exitCode=1;});
