/* NODE_PATH may point to a preinstalled Playwright. No production dependency. */
const {chromium}=require('playwright');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const {pathToFileURL}=require('node:url');
(async()=>{
  const out=path.join(__dirname,'test-results');fs.mkdirSync(out,{recursive:true});
  const browser=await chromium.launch({headless:true,channel:'msedge'});
  try{
    const context=await browser.newContext({viewport:{width:1440,height:1100},acceptDownloads:true});
    const page=await context.newPage();const errors=[];page.on('pageerror',e=>errors.push(e.message));
    await page.goto(pathToFileURL(path.join(__dirname,'../index.html')).href);
    await page.screenshot({path:path.join(out,'desktop.png'),fullPage:true});
    assert.match(await page.locator('h1').textContent(),/读懂项目/);
    const lessonIds=await page.evaluate(()=>COURSE.lessons.map(l=>l.id));
    for(const id of lessonIds){await page.goto(page.url().split('#')[0]+'#lesson='+id);await page.locator('#notes').waitFor();assert.ok((await page.locator('.reading section').count())>=6);assert.ok(await page.locator('.source').count());}
    await page.goto(page.url().split('#')[0]+'#lesson=L01');await page.locator('#notes').fill('我的概念卡：玩家档案跨 Pawn 保留。<script>window.injected=true</script>');
    await page.locator('#proof').fill('只做源码追踪；PIE 尚未验证。');await page.locator('[data-check="concept"]').check();await page.locator('[data-check="source"]').check();await page.locator('#review-check').check();
    await page.reload();assert.match(await page.locator('#notes').inputValue(),/玩家档案/);assert.equal(await page.locator('[data-check="concept"]').isChecked(),true);assert.equal(await page.locator('#lesson-count').textContent(),'2/4');
    const downloadPromise=page.waitForEvent('download');await page.locator('#lesson-export').click();const download=await downloadPromise;const backup=path.join(out,'backup.json');await download.saveAs(backup);
    await page.evaluate(()=>localStorage.removeItem('blaster-learning-v1'));await page.reload();assert.equal(await page.locator('#notes').inputValue(),'');await page.locator('#import').setInputFiles(backup);await page.waitForFunction(()=>document.querySelector('#notes').value.includes('玩家档案'));
    await page.locator('#notes').fill('更新后的概念卡 <script>window.injected=true</script>');
    await page.locator('#import').setInputFiles(backup);await page.waitForFunction(()=>document.querySelector('#notice').textContent.includes('已恢复'));assert.match(await page.locator('#notes').inputValue(),/更新后的概念卡/);
    const before=await page.locator('#notes').inputValue();await page.locator('#import').setInputFiles({name:'bad.json',mimeType:'application/json',buffer:Buffer.from('{"version":99,"lessons":{}}')});assert.equal(await page.locator('#notes').inputValue(),before);assert.match(await page.locator('#notice').textContent(),/恢复失败/);
    await page.goto(page.url().split('#')[0]+'#review');await page.locator('summary').first().click();assert.equal(await page.evaluate(()=>window.injected),undefined);assert.match(await page.locator('.review-note').first().textContent(),/<script>/);
    await page.goto(page.url().split('#')[0]+'#courses');await page.locator('#search').fill('ShotId');assert.ok(await page.locator('#results .lesson-row').count());await page.locator('#search').fill('不存在的课abcdef');assert.match(await page.locator('#results').textContent(),/没有匹配/);
    await page.goto(page.url().split('#')[0]+'#lesson=L11');await page.locator('#shot-time').fill('10050');await page.locator('#shot-time').dispatchEvent('input');assert.match(await page.locator('#lab-result').textContent(),/0.50/);await page.locator('.source summary').first().click();await page.screenshot({path:path.join(out,'lesson.png'),fullPage:true});
    await page.setViewportSize({width:390,height:844});await page.goto(page.url().split('#')[0]+'#home');await page.screenshot({path:path.join(out,'mobile.png'),fullPage:true});assert.ok(await page.locator('#mobile-import').isVisible());assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
    await page.goto(page.url().split('#')[0]+'#lesson=L18');assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
    await page.evaluate(()=>localStorage.setItem('blaster-learning-v1','broken'));await page.reload();assert.match(await page.locator('#notice').textContent(),/原数据未覆盖/);await page.locator('#notes').fill('临时记录');assert.equal(await page.evaluate(()=>localStorage.getItem('blaster-learning-v1')),'broken');
    assert.deepEqual(errors,[]);
    console.log(`PASS: ${lessonIds.length} lessons; file:// loading, persistence, backup restore, invalid backup, XSS text, search, SSR lab, 390px layout, corrupted storage; no page errors`);
  }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
