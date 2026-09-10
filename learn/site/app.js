'use strict';
const {lessons, docs, sources, builtAt} = window.COURSE;
const P = window.Progress;
const KEY = 'blaster-learning-v1';
const stageNames = ['建立项目映射','框架与网络权限','权威射击与回溯','输入、动画与 IK','Session、比赛与 HUD','构建验证与面试','选修 · 后续系统设计'];
const stageDesc = ['把 HSR 经验迁移到实时多人项目','谁拥有状态，谁请求，谁做最终裁决','从一次输入追到命中、伤害和重生','让身体、视角与手部姿态配合起来','从进入房间，到看到同步的比赛状态','用代码和验证证据支撑每一句表达','保留旧计划独有知识，明确未实现边界'];
const labels = ['理解原理','源码追踪','验证记录','面试复述'];
const ids = lessons.map(l => l.id);
const app = document.querySelector('#app');
const esc = v => String(v).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
let state = P.empty();
let persistenceBlocked = false;
function notice(message) { document.querySelector('#notice').textContent = message; }
try {
  const raw = localStorage.getItem(KEY);
  if (raw) state = P.validate(JSON.parse(raw), ids);
} catch (error) {
  persistenceBlocked = true;
  notice('本地记录无法读取，原数据未覆盖。本次可继续学习并导出备份。请先保留原浏览器数据，再恢复有效备份。');
}
function today() { const d = new Date(); return `${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`; }
function save() {
  if (persistenceBlocked) return false;
  try { localStorage.setItem(KEY, JSON.stringify(state)); return true; }
  catch { notice('浏览器未能保存（可能禁用存储或空间不足）。记录仍在当前页面，请立即导出备份。'); return false; }
}
function touch(id) {
  const item = state.lessons[id] ||= {};
  item.updated = new Date().toISOString();
  state.activity[today()] = (state.activity[today()] || 0) + 1;
  state.last = id;
  return save();
}
function bar(percent) { return `<div class="bar" role="progressbar" aria-valuenow="${percent}" aria-valuemin="0" aria-valuemax="100" aria-label="学习进度 ${percent}%"><span style="width:${percent}%"></span></div>`; }
function stageLessons(stage) { return lessons.filter(l => l.stage === stage); }
function nextLesson() { return lessons.find(l => l.id === state.last && P.count(state.lessons[l.id]) < 4) || lessons.find(l => l.stage < 6 && P.count(state.lessons[l.id]) < 4) || lessons[0]; }
function stageCard(stage) {
  const group = stageLessons(stage), s = P.stats(group,state), priority = stage === 0 ? '准备' : stage < 3 ? 'P0' : stage < 5 ? 'P1' : stage === 5 ? 'P2' : '选修';
  return `<a class="stage-card" href="#stage=${stage}"><div class="stage-top"><span>阶段 ${String(stage).padStart(2,'0')}</span><span class="chip ${priority.toLowerCase()}">${priority}</span></div><h3>${stageNames[stage]} ↗</h3><p>${stageDesc[stage]}</p>${bar(s.percent)}<div class="card-foot"><span>${s.done} / ${group.length} 课完成</span><span>${s.percent}%</span></div></a>`;
}
function heading(kicker,title,subtitle) { return `<div class="heading-row"><div><div class="eyebrow">${kicker}</div><h1>${title}</h1><div class="subtle">${subtitle}</div></div><span class="date-tag">${today().replaceAll('-',' / ')}</span></div>`; }
function home() {
  const core=lessons.filter(l=>l.stage<6), s=P.stats(core,state), next=nextLesson();
  const review=lessons.filter(l=>state.lessons[l.id]?.review).length;
  const notes=lessons.filter(l=>['notes','proof','speech'].some(f=>state.lessons[l.id]?.[f]?.trim())).length;
  app.innerHTML=heading('YOUR LEARNING JOURNEY','读懂项目，才能讲好项目。','从原理到源码，从验证到表达。今天，让一个知识点真正成为你的。')+
  `<div class="stats"><div class="stat"><div class="stat-label">主线学习进度</div><div class="stat-value">${s.percent}<small>%</small></div>${bar(s.percent)}<div class="stat-note">按四项学习产出计算</div></div><div class="stat"><div class="stat-label">已完成课程</div><div class="stat-value">${s.done}<small>/ ${core.length} 主线</small></div><div class="stat-note">另有 ${lessons.length-core.length} 节选修，不影响主线进度</div></div><div class="stat"><div class="stat-label">留下自己的理解</div><div class="stat-value">${notes}<small>份课程记录</small></div><div class="stat-note">概念 · 验证证据 · 面试回答</div></div><div class="stat"><div class="stat-label">待复习</div><div class="stat-value">${review}<small>个知识点</small></div><a href="#review" class="stat-note">去复习与记录 →</a></div></div>
  <div class="dashboard-middle"><div class="hero"><span class="hero-number">${next.id.slice(1)}</span><div class="eyebrow">${state.last?'CONTINUE LEARNING':'START SMALL, LEARN DEEPLY'}</div><h2>${esc(next.title)}</h2><p>阶段 ${next.stage} · ${next.tags.split(' ').slice(0,3).join(' / ')}<br>先理解，再追踪。不必答完检查题才继续。</p><a class="button primary" href="#lesson=${next.id}">${state.last?'继续上次学习':'开始第一课'} <span>→</span></a></div><div class="panel"><div class="panel-title">四项能力积累 <span>主线自评</span></div><div class="skills">${P.fields.map((f,i)=>{const n=core.filter(l=>state.lessons[l.id]?.[f]).length;return `<div class="skill-row"><span>${labels[i]}</span>${bar(Math.round(n/core.length*100))}<span>${n}/${core.length}</span></div>`;}).join('')}</div><div class="subtle">读过、验证过、能讲清，是不同的进展。</div></div></div>
  <div class="section-row"><h2>你的学习路线</h2><a href="#courses">查看全部 ${lessons.length} 节课程 →</a></div><div class="stage-grid">${[0,1,2,3,4,5].map(stageCard).join('')}</div><div class="section-row"><h2>延伸一步</h2><a href="#doc=learn/site/PLAN-RECONCILIATION.md">计划合并与覆盖说明 →</a></div><div class="stage-grid">${stageCard(6)}</div>
  <div class="panel bottom-panel"><div class="panel-title">最近 28 天的学习足迹 <span>修改笔记或更新自评时点亮</span></div><div class="heatmap">${Array.from({length:28},(_,i)=>{const d=new Date();d.setDate(d.getDate()-27+i);const day=`${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`;return `<span class="${state.activity[day]?'lit':''}" title="${day}：${state.activity[day]?'有学习记录':'暂无记录'}" aria-label="${day}：${state.activity[day]?'有学习记录':'暂无记录'}"></span>`;}).join('')}</div></div>`;
}
function row(l) { const n=P.count(state.lessons[l.id]);return `<a class="lesson-row" href="#lesson=${l.id}"><div><span class="lesson-index">${l.id} · 阶段 ${l.stage}</span><h3>${esc(l.title)}</h3><p>${esc(l.tags)} · 建议 ${l.minutes} 分钟起，验证可另安排</p></div><div><span class="chip">${n===4?'已完成':n?`${n}/4 进行中`:'待学习'}</span><div class="card-foot">${state.lessons[l.id]?.review?'待复习':'进入课程 →'}</div></div></a>`; }
function courses(stage) {
  app.innerHTML=heading('COURSE MAP',stage===undefined?'课程地图':stageNames[stage],stage===undefined?'先完成 P0，再进入动画与在线系统；所有课程随时可读。':stageDesc[stage])+`<div class="filters"><input id="search" type="search" aria-label="搜索课程" placeholder="搜索课程、概念或函数，如 ShotId、FABRIK…"><select id="filter" aria-label="进度筛选"><option value="all">所有状态</option><option value="todo">未完成</option><option value="done">已完成</option><option value="review">待复习</option></select></div><div id="results" class="lesson-list"></div>`;
  const filter=()=>{const q=document.querySelector('#search').value.toLowerCase().trim(),f=document.querySelector('#filter').value;const found=lessons.filter(l=>(stage===undefined||l.stage===stage)&&JSON.stringify(l).toLowerCase().includes(q)&&(f==='all'||f==='done'&&P.count(state.lessons[l.id])===4||f==='todo'&&P.count(state.lessons[l.id])<4||f==='review'&&state.lessons[l.id]?.review));document.querySelector('#results').innerHTML=found.length?found.map(row).join(''):'<div class="empty">没有匹配课程，试试其他关键词或状态。</div>';};
  document.querySelector('#search').oninput=filter;document.querySelector('#filter').onchange=filter;filter();
}
function source(ref) {
  const [path,symbol,line]=ref, data=sources[path];
  const lines=data.text.split('\n'), start=Math.max(0,line-4),end=Math.min(lines.length,line+65);
  return `<details class="source"><summary>${esc(path)} : ${line} · ${esc(symbol)}</summary><div class="source-meta">当前网站构建时的源码快照 · SHA256 ${data.sha256.slice(0,12)}<br>片段从第 ${start+1} 行开始；“查看完整源码”可阅读后续函数。</div><pre>${lines.slice(start,end).map((s,i)=>`<span class="source-line ${start+i+1===line?'marked':''}"><i>${start+i+1}</i>${esc(s)}</span>`).join('')}</pre><a class="button" href="#source=${encodeURIComponent(path)}">查看完整源码 →</a></details>`;
}
function lab() {return `<section><div class="section-label">互动实验 · 教学模型，不执行 UE 代码</div><h2>两张“照片”之间，目标在哪里？</h2><p>旧帧在 10.0 秒、x=0；新帧在 10.1 秒、x=100。移动射击时间，观察 Alpha 与插值位置。这里仅演示位置线性插值，实际源码 Blend 的是变换。</p><div class="lab-controls"><label for="shot-time">射击时间</label><input id="shot-time" type="range" min="9950" max="10150" step="1" value="10025"></div><div class="lab-stage"><div class="lab-caption">旧帧 x=0 ───────────────── 新帧 x=100</div><div class="lab-box" id="lab-box"></div></div><output id="lab-result" class="result"></output></section>`;}
function lesson(id) {
  const l=lessons.find(l=>l.id===id); if(!l) return missing();
  state.last=id;save();const item=state.lessons[id]||{},index=lessons.indexOf(l);
  app.innerHTML=`<div class="eyebrow">阶段 ${l.stage} / ${l.id} · ${l.stage===6?'选修设计':'项目实战'}</div><h1>${esc(l.title)}</h1><p class="subtle">${esc(l.tags)} · 先读本课，再按需要展开专题。学习记录是你的自评。</p><div class="boundary">项目边界：${esc(l.boundary)}</div><div class="lesson-layout"><article class="reading">
  <section><div class="section-label">01 / 先建立直觉</div><h2>它解决什么问题？</h2><p>${esc(l.idea)}</p></section>
  <section><div class="section-label">02 / 回到 BLASTER</div><h2>我们的项目具体怎样做？</h2><p>${esc(l.project)}</p><div class="flow">${l.flow.map((v,i)=>`${i?'<b>→</b>':''}<span>${esc(v)}</span>`).join('')}</div><h3>沿着真实入口读代码</h3>${l.refs.map(source).join('')}</section>
  ${l.id==='L11'?lab():''}
  <section><div class="section-label">03 / 亲手验证</div><h2>把理解变成可以检查的证据</h2><p>${esc(l.lab)}</p><label for="proof">验证记录</label><textarea id="proof" data-note="proof" maxlength="50000" placeholder="环境/日期：\n操作：\n预期：\n实际结果与日志/截图路径：\n尚未验证：">${esc(item.proof||'')}</textarea><span class="saved">输入后自动保存；需要长期保留时请导出备份。</span></section>
  <section><div class="section-label">04 / 面试表达</div><h2>${esc(l.question)}</h2><p>先试着用自己的话解释。也可以直接展开参考，再回来写你的版本。</p><details><summary>查看参考回答与边界</summary><p>${esc(l.answer)}</p></details><label for="speech">我的面试回答</label><textarea id="speech" data-note="speech" maxlength="50000" placeholder="结论 → 规则 → 项目函数与例子 → 取舍或未验证边界">${esc(item.speech||'')}</textarea></section>
  <section><div class="section-label">05 / 留下自己的理解</div><h2>概念卡与疑问</h2><label for="notes" class="subtle">用自己的话记录，不必抄写原文。</label><textarea id="notes" data-note="notes" maxlength="50000" placeholder="它解决什么问题？谁负责，谁不能负责？\n三个源码入口：\n我仍不理解的地方：">${esc(item.notes||'')}</textarea><div id="save-status" class="saved" role="status">记录保存在当前浏览器，可随时导出。</div></section>
  <section><div class="section-label">06 / 专题深读</div><h2>现有项目讲义与操作步骤</h2><p class="subtle">以下为仓库资料快照，保留原文及其历史验证范围。引擎源码阅读指引也在各专题中。</p>${l.docs.map(path=>`<details><summary>${esc(docs[path].title)}</summary><div class="document">${docs[path].html}</div></details>`).join('')}</section>
  <div class="pagination">${index?`<a class="button" href="#lesson=${lessons[index-1].id}">← 上一课</a>`:'<a class="button" href="#courses">课程地图</a>'}${index<lessons.length-1?`<a class="button primary" href="#lesson=${lessons[index+1].id}">下一课 →</a>`:'<a class="button primary" href="#review">回顾学习记录 →</a>'}</div></article>
  <aside class="lesson-aside"><div class="panel"><div class="panel-title">本课学习进度 <span id="lesson-count">${P.count(item)}/4</span></div><div id="lesson-bar" style="margin-top:15px">${bar(P.count(item)*25)}</div>${P.fields.map((f,i)=>`<label class="check"><input type="checkbox" data-check="${f}" ${item[f]?'checked':''}><span>${labels[i]}<small>${['能解释解决的问题与职责','追过入口、写入者和消费者','留下实测结果或明确的未测边界','用自己的话说过，并写下回答'][i]}</small></span></label>`).join('')}<div class="sidebar-line"></div><label class="check"><input type="checkbox" id="review-check" ${item.review?'checked':''}><span>加入待复习</span></label><p class="subtle">不设学习门禁。读完并不自动完成，勾选由你判断；随时可以取消。</p><button id="lesson-export">备份全部学习记录 ↗</button><a class="scope-nav" href="#stage=${l.stage}">返回阶段课程 →</a></div></aside></div>`;
  app.querySelectorAll('[data-note]').forEach(el=>el.addEventListener('input',()=>{const item=state.lessons[id]||={};item[el.dataset.note]=el.value;const ok=touch(id);document.querySelector('#save-status').textContent=ok?'已保存 · '+new Date().toLocaleTimeString():'当前页面内已记录；请导出备份以免丢失。';}));
  app.querySelectorAll('[data-check]').forEach(el=>el.addEventListener('change',()=>{const item=state.lessons[id]||={};item[el.dataset.check]=el.checked;touch(id);document.querySelector('#lesson-count').textContent=P.count(item)+'/4';document.querySelector('#lesson-bar').innerHTML=bar(P.count(item)*25);}));
  document.querySelector('#review-check').onchange=e=>{(state.lessons[id]||={}).review=e.target.checked;touch(id);};
  document.querySelector('#lesson-export').onclick=exportData;
  if(l.id==='L11') {const update=()=>{const t=Number(document.querySelector('#shot-time').value)/1000;const alpha=(t-10)/.1;const valid=t>=10;const x=Math.min(1,Math.max(0,alpha));document.querySelector('#lab-box').style.left=(8+x*84)+'%';document.querySelector('#lab-box').style.opacity=valid?1:.2;document.querySelector('#lab-result').textContent=!valid?`t=${t.toFixed(3)}：早于最旧历史帧，拒绝查询。`:t>10.1?`t=${t.toFixed(3)}：晚于最新帧但未超过 +0.1 秒，SampleFrame 返回最新帧，x=100。ServerFire 还会独立检查请求时间。`:`t=${t.toFixed(3)}：Alpha = (${t.toFixed(3)} − 10.0) / (10.1 − 10.0) = ${alpha.toFixed(2)}，x = ${(alpha*100).toFixed(1)}。`;};document.querySelector('#shot-time').oninput=update;update();}
}
function review() {
  const selected=lessons.filter(l=>{const d=state.lessons[l.id];return d&&(d.review||d.notes||d.proof||d.speech);});
  app.innerHTML=heading('REFLECT & REMEMBER','复习与记录','需要复习的课、自己的概念卡、验证证据和面试表达，都在这里。')+`<button id="review-export">导出学习备份 ↗</button><div class="lesson-list" style="margin-top:22px">${selected.length?selected.sort((a,b)=>Number(!!state.lessons[b.id].review)-Number(!!state.lessons[a.id].review)).map(l=>{const d=state.lessons[l.id];return `<div class="panel">${row(l)}${['notes','proof','speech'].map((f,i)=>d[f]?`<details><summary>${['概念卡','验证记录','我的面试回答'][i]}</summary><p class="review-note">${esc(d[f])}</p></details>`:'').join('')}<div class="subtle">上次修改：${esc(d.updated?new Date(d.updated).toLocaleString():'未记录')}</div></div>`;}).join(''):'<div class="empty">还没有记录。打开第一课，写下一句自己的理解。<p><a class="button primary" href="#lesson=L01">开始学习 →</a></p></div>'}</div>`;
  document.querySelector('#review-export').onclick=exportData;
}
function library() {
  app.innerHTML=heading('PROJECT KNOWLEDGE','项目资料库','主线用课程组织；原始讲义、计划与验证文档在这里完整保留。')+'<div class="filters"><input id="doc-search" type="search" aria-label="搜索资料" placeholder="搜索资料标题…"></div><div id="doc-results" class="lesson-list"></div>';
  const update=()=>{const q=document.querySelector('#doc-search').value.toLowerCase();document.querySelector('#doc-results').innerHTML=Object.entries(docs).filter(([path,d])=>(path+d.title).toLowerCase().includes(q)).map(([path,d])=>`<a class="lesson-row" href="#doc=${encodeURIComponent(path)}"><div><h3>${esc(d.title)}</h3><p>${esc(path)}</p></div><span>↗</span></a>`).join('')||'<div class="empty">没有匹配资料。</div>';};document.querySelector('#doc-search').oninput=update;update();
}
function missing() {app.innerHTML='<div class="empty">未找到该内容。<p><a class="button" href="#courses">返回课程地图</a></p></div>';}
function render() {
  const hash=location.hash.slice(1)||'home';const split=hash.indexOf('=');const type=split<0?hash:hash.slice(0,split);let value='';try{value=decodeURIComponent(hash.slice(split+1));}catch{return missing();}
  document.querySelectorAll('nav a').forEach(a=>a.classList.toggle('active',a.hash==='#'+(type==='lesson'||type==='stage'?'courses':type==='doc'||type==='source'?'library':type)));
  document.querySelector('#breadcrumb').textContent=({home:'学习总览',courses:'课程地图',stage:'阶段课程',lesson:'课程学习',review:'复习与记录',library:'项目资料库',doc:'项目讲义',source:'源码阅读'})[type]||'学习空间';
  if(type==='home') home();else if(type==='courses') courses();else if(type==='stage'&&/^\d$/.test(value)&&stageNames[+value]) courses(+value);else if(type==='lesson') lesson(value);else if(type==='review') review();else if(type==='library') library();else if(type==='doc'&&docs[value]) app.innerHTML=`<div class="pagination"><a class="button" href="#library">← 资料库</a></div><article class="document">${docs[value].html}</article>`;else if(type==='source'&&sources[value])app.innerHTML=`<h1>源码阅读</h1><p class="subtle">${esc(value)} · 构建时快照，代码变更后可重新生成</p><pre>${sources[value].text.split('\n').map((s,i)=>`<span class="source-line"><i>${i+1}</i>${esc(s)}</span>`).join('')}</pre>`;else missing();
  window.scrollTo(0,0);
}
function exportData() {
  const blob=new Blob([JSON.stringify(state,null,2)],{type:'application/json'});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download=`blaster-progress-${today()}.json`;a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);notice('备份已生成。它包含个人笔记，请保留在自己的备份位置；恢复时按每课修改时间合并。');
}
document.querySelector('#export').onclick=exportData;
document.querySelector('#mobile-export').onclick=exportData;
document.querySelector('#mobile-import').onclick=()=>document.querySelector('#import').click();
document.querySelector('#import').onchange=async e=>{
  const file=e.target.files[0];if(!file)return;
  try{
    if(file.size>8000000)throw Error('文件超过 8MB');
    const incoming=P.validate(JSON.parse(await file.text()),ids);
    // Re-read disk state because another open page may have newer records.
    const raw=localStorage.getItem(KEY);let current=state;
    if(raw){try{current=P.merge(P.validate(JSON.parse(raw),ids),state);}catch{/* Explicit recovery can replace malformed stored data. */}}
    const merged=P.merge(current,incoming);
    localStorage.setItem(KEY,JSON.stringify(merged));state=merged;persistenceBlocked=false;render();notice('备份已恢复：按每课最新修改时间合并，较新的本地记录已保留。');
  }catch(error){notice('恢复失败，当前记录未改变：'+error.message);}finally{e.target.value='';}
};
window.addEventListener('hashchange',render);
window.addEventListener('storage',e=>{if(e.key===KEY){persistenceBlocked=true;notice('另一个页面修改了学习记录。本页已暂停写入以避免覆盖；请导出本页备份，再刷新并按需要恢复合并。');}});
document.querySelector('#stage-nav').innerHTML=stageNames.map((n,i)=>`<a href="#stage=${i}"><i>${String(i).padStart(2,'0')}</i>${n}</a>`).join('');
document.querySelector('#snapshot-date').textContent=builtAt.slice(0,10);
render();
