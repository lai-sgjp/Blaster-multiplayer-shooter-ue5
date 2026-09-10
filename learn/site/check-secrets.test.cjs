const test=require('node:test');
const assert=require('node:assert/strict');
const {inspect}=require('./check-secrets.cjs');
test('secret guard detects staged credential paths and token patterns without echoing values',()=>{
  const credential='sk-'+ 'a'.repeat(32);
  assert.ok(inspect('.env.local','').length);
  assert.ok(inspect('nested/credentials.json','{}').length);
  const found=inspect('source.js','const x="'+credential+'";');assert.equal(found.length,1);assert.ok(!JSON.stringify(found).includes(credential));
  assert.deepEqual(inspect('.env.example','API_KEY=YOUR_KEY_HERE'),[]);
  assert.deepEqual(inspect('settings.js','const apiKey = input.value;'),[]);
  assert.deepEqual(inspect('avatar.png','\0image'),[]);
});
