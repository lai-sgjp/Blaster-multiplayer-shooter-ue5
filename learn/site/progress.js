/* Pure state rules shared by the browser and node tests. */
(function(root) {
  'use strict';
  const fields = ['concept', 'source', 'evidence', 'interview'];
  function empty() { return {version: 1, lessons: {}, last: '', activity: {}}; }
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
  const api = {fields, empty, validate, count, stats, merge};
  if (typeof module !== 'undefined') module.exports = api;
  else root.Progress = api;
})(typeof window !== 'undefined' ? window : globalThis);
