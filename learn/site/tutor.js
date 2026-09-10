(function(){
  'use strict';
  const learning=window.BlasterLearning;
  const escape=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  let session=null,busy=false,controller=null,lastFocus=null;
  const host=document.createElement('div');host.id='tutor-root';
  host.innerHTML=`<button id="tutor-launch" aria-controls="tutor-panel" aria-expanded="false" title="与蕾米埃尔一起学习"><img src="site/assets/remiel.png" alt="粉发紫翼的蕾米埃尔"><span>问问蕾米埃尔<small>你的项目学习共犯 ↗</small></span></button>
  <section id="tutor-panel" hidden aria-labelledby="tutor-title" role="dialog"><header class="tutor-heading"><div><span class="tutor-eyebrow">BLASTER LAB / VIRTUAL TUTOR</span><h2 id="tutor-title">蕾米埃尔 <small>你的学习共犯</small></h2></div><button id="tutor-close" aria-label="收起助教">×</button></header>
  <div class="tutor-intro"><img src="site/assets/remiel.png" alt="蕾米埃尔角色立绘"><div><p>共犯，把困惑给我。<br>我们顺着代码，一起拆穿它。</p><span id="tutor-connection">正在检查本机连接…</span><button id="tutor-settings-toggle">⚙ API 设置</button></div></div>
  <div id="tutor-status" role="status" aria-live="polite"></div>
  <div class="tutor-scroll">
    <form id="tutor-settings" hidden autocomplete="off"><h3>连接你的 API</h3><label>API Base URL<input id="tutor-base" type="url" required placeholder="https://你的服务地址/v1" autocomplete="off"></label><label>模型名称<input id="tutor-model" required placeholder="填写服务商提供的模型 ID" maxlength="150" autocomplete="off"></label><label>接口格式<select id="tutor-api-mode"><option value="chat">Chat Completions · 通用兼容</option><option value="chat-modern">Chat Completions · 新版 token 参数</option><option value="responses">Responses API</option></select></label><label>API Key<input id="tutor-key" type="password" placeholder="尚未配置；保存后不会回显" maxlength="4096" autocomplete="new-password" spellcheck="false"></label><p class="tutor-hint">Key 仅发给本机服务，由 Windows 用户加密保护，保存在仓库外。留空保留当前 Key；更换地址须重新填写。</p><div class="tutor-actions"><button type="submit" class="primary">保存设置</button><button id="tutor-clear-key" type="button">移除已存 Key</button></div></form>
    <details class="tutor-persona"><summary>她是谁？ · 角色卡</summary><p>粉发紫翼，跨越时代的见闻藏在从容的笑里。她聪明、爱开一点玩笑，也会认真指出你的漏洞。称你为“共犯”，但不会帮你把不会的知识蒙混过关。角色演绎依据你提供的图片，并非官方角色服务。</p></details>
    <div id="tutor-course"></div><div id="tutor-history" aria-label="课程助教记录"></div>
  </div>
  <form id="tutor-chat"><label for="tutor-question" class="tutor-hint">你的问题（Enter 发送，Shift+Enter 换行）</label><textarea id="tutor-question" maxlength="8000" rows="3" placeholder="例如：Owner 和 Authority 到底差在哪？"></textarea><div class="tutor-actions"><button id="tutor-send" type="submit" class="primary">发送问题 ↗</button><button id="tutor-cancel" type="button" hidden>取消等待</button></div><div class="tutor-evaluate"><select id="tutor-answer-kind" aria-label="选择待评价内容"><option value="speech">我的面试回答</option><option value="notes">概念卡与疑问</option><option value="proof">验证记录的表述</option></select><button id="tutor-evaluate" type="button">评价当前回答</button></div><p class="tutor-hint">发送时仅提交当前课摘要/源码片段、所选回答或问题与最近对话给已配置服务；不会自动上传全部笔记。评价自动留档，进度需点击采纳。</p></form></section>`;
  document.body.append(host);
  const $=s=>host.querySelector(s),panel=$('#tutor-panel');
  const status=message=>{$('#tutor-status').textContent=message;};
  function refreshRecord(){
    const lesson=learning.currentLesson();$('#tutor-course').textContent=lesson?`${lesson.id} · ${lesson.title}`:'先打开一节课程，再一起读代码。';
    const record=lesson?learning.record(lesson.id):{};
    const messages=(record.tutorMessages||[]).map(m=>`<div class="tutor-message ${m.role}"><b>${m.role==='user'?'你':'蕾米埃尔'}</b><p>${escape(m.text)}</p></div>`).join('');
    const evaluations=(record.assessments||[]).slice().reverse().map(a=>`<article class="tutor-assessment"><div class="tutor-score"><b>回答评价 ${a.evaluation.score}<small>/100</small></b><span>${escape(new Date(a.createdAt).toLocaleString())}</span></div><p>${escape(a.evaluation.reply)}</p><h4>说对的地方</h4><ul>${a.evaluation.strengths.map(x=>`<li>${escape(x)}</li>`).join('')}</ul><h4>需要补上的地方</h4><ul>${a.evaluation.gaps.map(x=>`<li>${escape(x)}</li>`).join('')}</ul><p><b>下一步：</b>${escape(a.evaluation.nextStep)}</p><details><summary>查看当时提交的回答</summary><p>${escape(a.answer)}</p></details><p class="tutor-hint">建议补充：${[['concept','理解原理'],['source','源码追踪'],['interview','面试复述'],['review','待复习']].filter(([f])=>a.evaluation.suggestion[f]).map(([,label])=>label).join('、')||'暂不新增勾选'}。保留已有自评；不修改验证勾选。</p><button data-adopt="${escape(a.id)}" data-lesson="${lesson.id}" ${a.adoptedAt?'disabled':''}>${a.adoptedAt?'已采纳进度建议':'采纳这些进度建议'}</button><small class="tutor-hint">模型：${escape(a.model)} · AI 辅助意见</small></article>`).join('');
    $('#tutor-history').innerHTML=messages+evaluations||'<div class="tutor-empty">不懂也没关系。先问一个具体问题，或者写好你的回答后让我看看。</div>';
    const ready=!!(session?.configured&&lesson&&!busy);$('#tutor-send').disabled=!ready;$('#tutor-evaluate').disabled=!ready;
  }
  async function connect(){
    if(location.protocol==='file:'){$('#tutor-connection').textContent='离线阅读模式';status('在线助教需双击 learn/启动学习助教.cmd。原离线记录请先导出，再在本机网页恢复；两个地址的浏览器存储不同。');refreshRecord();return;}
    try{const response=await fetch('/api/session',{cache:'no-store'});if(!response.ok)throw Error();session=await response.json();if(session.app!=='BlasterLabTutor')throw Error();fillSettings();$('#tutor-connection').textContent=session.configured?'本机服务已连接 · API 待请求时验证':'本机服务已连接 · 请配置 API';if(!session.configured)$('#tutor-settings').hidden=false;}
    catch{session=null;$('#tutor-connection').textContent='本机服务未连接';status('请运行“启动学习助教.cmd”，再重新打开面板。离线课程与笔记仍然可用。');}
    refreshRecord();
  }
  function fillSettings(){const c=session.settings;$('#tutor-base').value=c.baseUrl;$('#tutor-model').value=c.model;$('#tutor-api-mode').value=c.apiMode;$('#tutor-key').value='';$('#tutor-key').placeholder=c.hasKey?'已保存（不回显）；留空保留':'在这里输入你的 API Key';}
  async function api(route,body){
    if(!session)throw Error('先启动本机助教服务');
    const response=await fetch(route,{method:'POST',headers:{'Content-Type':'application/json','X-Blaster-Token':session.token},body:JSON.stringify(body),signal:controller?.signal});
    const data=await response.json();if(!response.ok)throw Error(data.error||'请求失败，请重试');return data;
  }
  function setBusy(value){busy=value;$('#tutor-cancel').hidden=!value;$('#tutor-settings').querySelectorAll('input,select,button').forEach(el=>el.disabled=value);refreshRecord();}
  async function saveSettings(clearKey=false){
    const input={baseUrl:$('#tutor-base').value.trim(),model:$('#tutor-model').value.trim(),apiMode:$('#tutor-api-mode').value,apiKey:$('#tutor-key').value,clearKey};
    try{setBusy(true);const data=await api('/api/settings',input);session={...session,...data};fillSettings();$('#tutor-connection').textContent=data.configured?'设置已保存 · 发送问题时验证 API':'Key 已移除';status(clearKey?'已移除本机保存的 Key。':'设置已保存。现在可以打开课程提问；这一步未调用付费模型。');if(!clearKey)$('#tutor-settings').hidden=true;}
    catch(e){status(e.message);}finally{input.apiKey='';$('#tutor-key').value='';setBusy(false);}
  }
  async function send(mode){
    if(busy)return;
    const lesson=learning.currentLesson();if(!lesson){status('请先从课程地图打开一节课。');return;}
    const record=learning.record(lesson.id),kind=$('#tutor-answer-kind').value;
    const message=$('#tutor-question').value.trim(),answer=mode==='evaluate'?(record[kind]||'').trim():'';
    if(mode==='chat'&&!message||mode==='evaluate'&&!answer){status(mode==='chat'?'先写下你想问的问题。':'当前所选回答为空。请先在课程中写下你的回答，再请我评价。');return;}
    controller=new AbortController();setBusy(true);status(`${lesson.id} · 正在${mode==='evaluate'?'阅读并评价你的回答':'思考你的问题'}…`);
    try{
      const history=mode==='chat'?(record.tutorMessages||[]).slice(-6).map(m=>({role:m.role,content:m.text.slice(0,8000)})):[];
      const data=await api('/api/message',{lessonId:lesson.id,mode,message:mode==='chat'?message:'',answer,answerKind:kind,history});
      let saved;
      if(mode==='evaluate')saved=learning.addAssessment(lesson.id,{id:crypto.randomUUID(),answer,answerKind:kind,model:data.model,createdAt:data.createdAt,evaluation:data.evaluation,adoptedAt:''});
      else{const first=learning.addMessage(lesson.id,{role:'user',text:message,createdAt:data.createdAt});saved=learning.addMessage(lesson.id,{role:'assistant',text:data.reply,createdAt:data.createdAt})&&first;if($('#tutor-question').value.trim()===message)$('#tutor-question').value='';}
      status(saved?`${lesson.id} · ${mode==='evaluate'?'评价已保存，尚未更改完成进度':'对话已保存'}。每课保留最近 5 次评价和 6 轮问答，可随学习备份导出。`:'回答已留在本页，但浏览器保存失败，请立即导出学习备份。');
    }catch(e){status(e.name==='AbortError'?'已取消等待。模型服务可能仍已计费；没有修改进度或记录不完整回答。':`没有保存不完整回答：${e.message}。可再次点击重试。`);}
    finally{controller=null;setBusy(false);}
  }
  function open(){lastFocus=document.activeElement;panel.hidden=false;$('#tutor-launch').setAttribute('aria-expanded','true');connect();$('#tutor-close').focus();}
  function close(){panel.hidden=true;$('#tutor-launch').setAttribute('aria-expanded','false');lastFocus?.focus();}
  $('#tutor-launch').onclick=()=>panel.hidden?open():close();$('#tutor-close').onclick=close;
  $('#tutor-settings-toggle').onclick=()=>{$('#tutor-settings').hidden=!$('#tutor-settings').hidden;if(!session)connect();};
  $('#tutor-settings').onsubmit=e=>{e.preventDefault();saveSettings();};$('#tutor-clear-key').onclick=()=>saveSettings(true);
  $('#tutor-chat').onsubmit=e=>{e.preventDefault();send('chat');};$('#tutor-evaluate').onclick=()=>send('evaluate');
  $('#tutor-cancel').onclick=()=>controller?.abort();
  $('#tutor-question').onkeydown=e=>{if(e.key==='Enter'&&!e.shiftKey&&!e.isComposing){e.preventDefault();if(!$('#tutor-send').disabled)send('chat');}};
  host.addEventListener('click',e=>{const button=e.target.closest('[data-adopt]');if(!button)return;try{const saved=learning.adopt(button.dataset.lesson,button.dataset.adopt);refreshRecord();status(saved?'已采纳建议并保存。验证记录仍由你亲自确认。':'建议已采纳到本页，但保存失败，请导出备份。');}catch(error){status(error.message);}});
  document.addEventListener('keydown',e=>{if(e.key==='Escape'&&!panel.hidden)close();});
  window.addEventListener('hashchange',refreshRecord);
  function decorate(){
    for(const kind of ['speech','notes','proof']){
      const field=document.getElementById(kind);
      if(field&&!field.parentElement.querySelector('[data-tutor-open]')){
        const button=document.createElement('button');button.type='button';button.className='tutor-inline';button.dataset.tutorOpen=kind;button.textContent='让蕾米埃尔看看这份回答 ↗';field.after(button);
      }
    }
    refreshRecord();
  }
  document.addEventListener('click',e=>{const button=e.target.closest('[data-tutor-open]');if(button){$('#tutor-answer-kind').value=button.dataset.tutorOpen;open();}});
  window.addEventListener('blaster:render',decorate);
  decorate();
  refreshRecord();
})();
