/* Pure state rules shared by the browser and node tests. */
(function(root) {
  'use strict';
  const fields = ['concept', 'source', 'evidence', 'interview'];
  function empty() { return {version: 1, lessons: {}, last: '', activity: {}}; }
  function tutorRecords(data) {
    const messages = data.tutorMessages || [], assessments = data.assessments || [];
    if (!Array.isArray(messages) || messages.length > 12 || !Array.isArray(assessments) || assessments.length > 5) throw Error('助教记录数量超出限制');
    const str=(v,max)=>{if(typeof v!=='string'||v.length>max)throw Error('助教记录文本无效');return v;};
    const date=v=>{str(v,40);if(!Number.isFinite(Date.parse(v)))throw Error('助教记录日期无效');return v;};
    return {
      tutorMessages:messages.map(m=>{if(!m||!['user','assistant'].includes(m.role))throw Error('助教消息角色无效');return {role:m.role,text:str(m.text,24000),createdAt:date(m.createdAt)};}),
      assessments:assessments.map(a=>{
        if(!a||!a.evaluation)throw Error('助教评价无效');const e=a.evaluation;
        if(!Number.isInteger(e.score)||e.score<0||e.score>100)throw Error('助教分数无效');
        const list=v=>{if(!Array.isArray(v)||v.length>8)throw Error('助教评价列表无效');return v.map(s=>str(s,1000));};
        const suggestion={};for(const f of ['concept','source','interview','review']){if(typeof e.suggestion?.[f]!=='boolean')throw Error('助教进度建议无效');suggestion[f]=e.suggestion[f];}
        return {id:str(a.id,100),answer:str(a.answer,12000),answerKind:str(a.answerKind||'speech',30),model:str(a.model,150),createdAt:date(a.createdAt),adoptedAt:a.adoptedAt?date(a.adoptedAt):'',evaluation:{reply:str(e.reply,8000),score:e.score,strengths:list(e.strengths),gaps:list(e.gaps),nextStep:str(e.nextStep,2000),suggestion}};
      })
    };
  }
  function validate(value, ids) {
    if (!value || value.version !== 1 || !value.lessons || Array.isArray(value.lessons) || typeof value.lessons !== 'object') throw Error('备份格式或版本不兼容');
    const out = empty();
    for (const [id, data] of Object.entries(value.lessons)) {
      if (!ids.includes(id)) throw Error('备份包含未知课程：' + id);
      if (!data || typeof data !== 'object' || Array.isArray(data)) throw Error('课程记录格式错误');
      const item = {};
      for (const field of fields) {
        if (data[field] !== undefined && typeof data[field] !== 'boolean') throw Error('进度字段必须为布尔值');
        item[field] = data[field] === true;
      }
      for (const field of ['notes', 'proof', 'speech']) {
        if (data[field] !== undefined && (typeof data[field] !== 'string' || data[field].length > 50000)) throw Error('笔记格式错误或过长');
        item[field] = data[field] || '';
      }
      if (data.review !== undefined && typeof data.review !== 'boolean') throw Error('复习标记无效');
      item.review = data.review === true;
      if (data.updated && (typeof data.updated !== 'string' || !Number.isFinite(Date.parse(data.updated)))) throw Error('记录日期无效');
      item.updated = data.updated || '';
      Object.assign(item,tutorRecords(data));
      out.lessons[id] = item;
    }
    if (value.last && !ids.includes(value.last)) throw Error('上次学习课程无效');
    out.last = value.last || '';
    if (value.activity !== undefined && (!value.activity || typeof value.activity !== 'object' || Array.isArray(value.activity))) throw Error('活动记录无效');
    for (const [day, n] of Object.entries(value.activity || {})) {
      if (!/^\d{4}-\d{2}-\d{2}$/.test(day) || !Number.isSafeInteger(n) || n < 0) throw Error('活动记录无效');
      out.activity[day] = n;
    }
    return out;
  }
  function count(item = {}) { return fields.filter(f => item[f] === true).length; }
  function stats(lessons, state) {
    const units = lessons.reduce((n, l) => n + count(state.lessons[l.id]), 0);
    return {units, percent: lessons.length ? Math.round(units / (lessons.length * 4) * 100) : 0, done: lessons.filter(l => count(state.lessons[l.id]) === 4).length};
  }
  function merge(current, incoming) {
    const out = {version: 1, last: incoming.last || current.last, lessons: {...current.lessons}, activity: {...current.activity}};
    for (const [id, item] of Object.entries(incoming.lessons)) {
      if (!out.lessons[id] || Date.parse(item.updated || '1970-01-01') > Date.parse(out.lessons[id].updated || '1970-01-01')) out.lessons[id] = item;
    }
    for (const [day, n] of Object.entries(incoming.activity)) out.activity[day] = Math.max(out.activity[day] || 0, n);
    return out;
  }
  const api = {fields, empty, validate, count, stats, merge, tutorRecords};
  if (typeof module !== 'undefined') module.exports = api;
  else root.Progress = api;
})(typeof window !== 'undefined' ? window : globalThis);
