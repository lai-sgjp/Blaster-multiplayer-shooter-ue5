'use strict';
// Reports filenames/line numbers only. Never print a matching secret.
const {execFileSync}=require('node:child_process');
const path=require('node:path');
function inspect(name,raw){
  const violations=[];
  if(/(^|\/)(\.env(?:\..+)?|secrets\.json|credentials\.json|api-keys\.json|tutor\.config\.json|config\.local\.json)$/.test(name)&&!/(\.example|\.template)$/.test(name))violations.push('secret configuration file');
  if(raw.includes('\0'))return violations;
  const patterns=[/\bsk-(?:proj-|ant-)?[A-Za-z0-9_-]{20,}/,/\b(?:ghp_|github_pat_)[A-Za-z0-9_]{20,}/,/-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----/,/\bAKIA[A-Z0-9]{16}\b/,/(?:api[_-]?key|access[_-]?token|client[_-]?secret)\s*["']?\s*[:=]\s*["'][A-Za-z0-9_+\/-]{24,}["']/i];
  raw.split(/\r?\n/).forEach((line,i)=>{if(patterns.some(p=>p.test(line)))violations.push('possible credential at line '+(i+1));});
  return violations;
}
function scan(){
  const root=execFileSync('git',['rev-parse','--show-toplevel'],{encoding:'utf8'}).trim();
  const names=execFileSync('git',['diff','--cached','--name-only','--diff-filter=ACMR','-z'],{cwd:root,encoding:'utf8'}).split('\0').filter(Boolean);let failed=false;
  for(const name of names){const raw=execFileSync('git',['show',':'+name],{cwd:root,maxBuffer:32*1024*1024}).toString('utf8');for(const issue of inspect(name,raw)){console.error(`${name}: ${issue} (content redacted)`);failed=true;}}
  if(failed){console.error('Commit blocked: remove secrets from the index and keep them outside the repository.');process.exitCode=1;}else console.log(`Secret guard: checked ${names.length} staged files; no known credential pattern found.`);
}
if(require.main===module)scan();
module.exports={inspect};
