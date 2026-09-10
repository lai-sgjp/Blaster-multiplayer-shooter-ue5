const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const crypto = require('node:crypto');
const P = require('./progress.js');
const fixture = {window:{}};
vm.runInNewContext(fs.readFileSync(path.join(__dirname,'content.js'),'utf8'),fixture);
const course=fixture.window.COURSE, ids=course.lessons.map(l=>l.id);

test('partial learning counts units, not an entire completed lesson',()=>{
  const state=P.empty();state.lessons.L01={concept:true,source:true};
  assert.deepEqual(P.stats([{id:'L01'},{id:'L02'}],state),{units:2,percent:25,done:0});
  state.lessons.L01={concept:true,source:true,evidence:true,interview:true};
  assert.equal(P.stats([{id:'L01'},{id:'L02'}],state).done,1);
  assert.equal(P.stats([],state).percent,0);
});
test('backup roundtrip retains Unicode notes and explicit unchecked state',()=>{
  const s=P.empty();s.lessons.L01={concept:false,notes:'中文\n<script>not executable</script>',updated:'2026-09-10T00:00:00Z'};
  const restored=P.validate(JSON.parse(JSON.stringify(s)),ids);
  assert.equal(restored.lessons.L01.notes,s.lessons.L01.notes);
  assert.equal(restored.lessons.L01.concept,false);
});
test('malformed backups are rejected, including prototype and oversized fields',()=>{
  for(const bad of [null,{version:2,lessons:{}},{version:1,lessons:[]},{version:1,lessons:{L01:{concept:'yes'}}},{version:1,lessons:{L01:{notes:'x'.repeat(50001)}}},JSON.parse('{"version":1,"lessons":{"__proto__":{}}}'),{version:1,lessons:{},activity:{today:1}},{version:1,lessons:{L01:{updated:'bad'}}}])assert.throws(()=>P.validate(bad,ids));
});
test('merging an older backup never overwrites newer notes or unchecks',()=>{
  const a=P.empty(),b=P.empty();a.lessons.L01={concept:false,notes:'new',updated:'2026-09-10T10:00:00Z'};b.lessons.L01={concept:true,notes:'old',updated:'2026-09-09T10:00:00Z'};
  assert.equal(P.merge(a,b).lessons.L01.notes,'new');assert.equal(P.merge(a,b).lessons.L01.concept,false);
  b.lessons.L02={notes:'restored',updated:'2026-09-08T10:00:00Z'};assert.equal(P.merge(a,b).lessons.L02.notes,'restored');
});
test('all planned stages have substantive teaching, references and embedded docs',()=>{
  assert.equal(new Set(ids).size,ids.length);
  for(let i=0;i<7;i++)assert.ok(course.lessons.some(l=>l.stage===i));
  for(const l of course.lessons){for(const f of ['idea','project','answer','lab'])assert.ok(l[f].length>35,`${l.id} ${f}`);assert.ok(l.refs.length);assert.ok(l.docs.length);for(const d of l.docs)assert.ok(course.docs[d],d);}
});
test('source anchors and hashes match the actual checkout, not invented code',()=>{
  for(const [file,source] of Object.entries(course.sources)){
    const raw=fs.readFileSync(path.join(__dirname,'../..',file));
    assert.equal(crypto.createHash('sha256').update(raw).digest('hex'),source.sha256,file);
  }
  for(const l of course.lessons)for(const [file,symbol,line] of l.refs)assert.ok(course.sources[file].text.split('\n')[line-1].includes(symbol),`${l.id} ${symbol}`);
});
test('embedded document links resolve to bundled documents or source',()=>{
  for(const [file,doc] of Object.entries(course.docs))for(const match of doc.html.matchAll(/href="#(doc|source)=([^"]+)"/g))assert.ok((match[1]==='doc'?course.docs:course.sources)[match[2]],`${file}: ${match[2]}`);
});
