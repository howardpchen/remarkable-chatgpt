#!/usr/bin/env python3
"""No network or real credentials: exercise CLI policy, persistence and a real TTY."""
import hashlib,json,os,pathlib,pty,select,subprocess,sys,tempfile,time
binary=str(pathlib.Path(sys.argv[1]).resolve())
key='synthetic_key_NOT_A_CREDENTIAL'
keyid=hashlib.sha256(key.encode()).hexdigest()[:16]
cases=0
with tempfile.TemporaryDirectory() as tmp:
    base=pathlib.Path(tmp); configdir=base/'config';configdir.mkdir(mode=0o700)
    config=configdir/'config.json'; state=base/'state';state.mkdir(mode=0o700)
    def write(path,value):
        path.write_text(json.dumps(value));path.chmod(0o600)
    def status():
        global cases
        p=subprocess.run([binary,'key','status','--json','--config',str(config),'--state-dir',str(state)],capture_output=True)
        assert p.returncode==0,p.stderr
        assert key.encode() not in p.stdout+p.stderr
        d=json.loads(p.stdout);assert not d['networkCheck'] and d['providerAccountBalance']=='unknown'
        cases+=1;return d
    def snapshot():return {str(p.relative_to(state)):p.read_bytes() for p in state.rglob('*') if p.is_file()}
    assert status()['key']['state']=='missing'
    write(config,dict(api_key=key,model='gpt-6-luna',unrelated={'keep':True}))
    assert status()['key']['identifier']=='sha256:'+keyid
    policy=dict(schema=1,model='gpt-6-luna',totalNanoUsd=5000000000,firstRequestId=1)
    write(state/'budget-policy.json',policy)
    d=status();assert d['allowance']['availableUsd']==5 and d['allowance']['canAffordNextRequest']
    for i in range(1,5):(state/str(i)).mkdir(mode=0o700)
    d=status();assert d['allowance']['attempts']==4 and d['allowance']['unsettledAttempts']==4
    assert abs(d['allowance']['availableUsd']-(5-4*.25421))<1e-9
    cost=state/'1'/'BUDGET_COST';cost.write_text('1000000\n');cost.chmod(0o600)
    d=status();assert d['allowance']['unsettledAttempts']==3
    cost.write_text('invalid\n');assert status()['allowance']['state']=='unavailable-or-unprovisioned'
    cost.write_text('1000000\n')
    observation=state/'provider-status.json'
    for code,http,expected in [('invalid_api_key',401,'authentication'),('insufficient_quota',429,'billing'),('rate_limit_exceeded',429,'rate limit'),('',200,'not checked'),('',403,'permissions'),('',503,'failed')]:
        write(observation,dict(schema=1,keyId=keyid,httpStatus=http,code=code,observedAt='2026-10-27T12:00:00Z'))
        d=status();assert d['provider']['state']=='last-observed' and not d['provider']['freshVerification']
        assert expected in ' '.join(d['guidance']),d
    write(observation,dict(schema=1,keyId='a'*16,httpStatus=401,code='invalid_api_key',observedAt='2026-10-27T12:00:00Z'))
    assert status()['provider']['state']=='observation-for-different-key'
    write(observation,dict(schema=1,keyId=keyid,httpStatus=401,code=key,observedAt='2026-10-27T12:00:00Z'))
    assert status()['provider']['state']=='observation-unavailable'
    for i in range(5,22):(state/str(i)).mkdir(mode=0o700)
    d=status();assert d['allowance']['state']=='unavailable-or-unprovisioned' # overspent ledger fails closed
    (state/'21').rmdir()
    d=status();assert d['allowance']['state']=='exhausted' and not d['allowance']['canAffordNextRequest']
    saved=snapshot()
    p=subprocess.run([binary,'key','remove','--config',str(config)],capture_output=True);assert p.returncode==0
    assert 'api_key' not in json.loads(config.read_text()) and json.loads(config.read_text())['unrelated']=={'keep':True}
    assert snapshot()==saved and status()['key']['state']=='missing'
    # Real controlling terminal: hidden entry and no secret in terminal output.
    pid,fd=pty.fork()
    if pid==0:os.execv(binary,[binary,'key','set','--config',str(config)])
    output=b'';sent=False;deadline=time.monotonic()+8
    while time.monotonic()<deadline:
        if not select.select([fd],[],[],.1)[0]:continue
        try:block=os.read(fd,4096)
        except OSError:break
        if not block:break
        output+=block
        if b'test): ' in output and not sent:os.write(fd,key.encode()+b'\n');sent=True
    _,exitcode=os.waitpid(pid,0);os.close(fd)
    assert exitcode==0 and sent and key.encode() not in output,output
    assert json.loads(config.read_text())['api_key']==key and snapshot()==saved
    assert status()['key']['state']=='configured'
    config.chmod(0o644);assert status()['key']['state']=='invalid-or-unsafe';config.chmod(0o600)
    other=configdir/'other';config.rename(other);config.symlink_to(other)
    assert status()['key']['state']=='invalid-or-unsafe'
    assert subprocess.run([binary,'key','remove','--config',str(config)],capture_output=True).returncode!=0
    assert json.loads(other.read_text())['api_key']==key
    # Safe state root refuses symlink ancestors; status does not create any files.
    assert snapshot()==saved
print(f'PASS CLI: {cases} status cases plus hidden set/remove, preserved ledger/settings and unsafe-path rejection')

# Reviewed legacy reconciliation is preview-first, explicit-ID, offline-only.
with tempfile.TemporaryDirectory() as tmp:
    root=pathlib.Path(tmp);write(root/'budget-policy.json',policy)
    (root/'1').mkdir(mode=0o700);(root/'2').mkdir(mode=0o700)
    (root/'2'/'START_CONSUMED').write_text('consumed\n')
    cmd=[binary,'budget','reconcile-unsent','--state-dir',str(root),'--request-id','1']
    before={str(p.relative_to(root)):p.read_bytes() for p in root.rglob('*') if p.is_file()}
    assert subprocess.run(cmd,capture_output=True).returncode==0
    assert before=={str(p.relative_to(root)):p.read_bytes() for p in root.rglob('*') if p.is_file()}
    assert subprocess.run(cmd+['--apply'],capture_output=True).returncode==2
    assert subprocess.run(cmd+['--apply','--runtime-stopped'],capture_output=True).returncode==0
    assert (root/'1'/'BUDGET_RELEASE').read_text()=='legacy-no-start-v1\n'
    assert subprocess.run(cmd+['--apply','--runtime-stopped'],capture_output=True).returncode==1
    cmd[-1]='2';assert subprocess.run(cmd+['--apply','--runtime-stopped'],capture_output=True).returncode==1
    assert not (root/'2'/'BUDGET_RELEASE').exists()
    assert json.loads((root/'budget-policy.json').read_text())==policy
print('PASS reconciliation: read-only preview, explicit offline apply, repeat/started refusal, preserved history/policy')
