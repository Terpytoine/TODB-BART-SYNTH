/* The world is decorative: no MIDI messages, audio state or synth parameters are sent here. */
const SCENES = [
  {id:'platform-a',name:'Platform A',place:'San Leandro · northbound',asset:'scene-platform-a.png',platform:2,pose:0,accent:'#a376c5',spots:[['Station lights',.92,.37,'Three lines serve San Leandro: Blue, Green and Orange.'],['Platform session',.17,.56,'A beat between trains. One more idea before boarding.']]},
  {id:'platform-b',name:'Platform B',place:'San Leandro · southbound',asset:'scene-platform-b.png',platform:1,pose:1,accent:'#cea966',spots:[['Southbound',.13,.52,'Dublin/Pleasanton or Berryessa/North San Jose. Check the destination.'],['Late-night take',.88,.72,'The headphones stay on. The station keeps moving.']]},
  {id:'concourse',name:'San Leandro Station',place:'Ground-level concourse',asset:'scene-concourse.png',platform:2,pose:2,accent:'#9277b2',spots:[['Station entry',.55,.56,'Under the elevated platforms, one more idea before the train.'],['TODB sticker',.12,.42,'TWON ON DA BEAT · made in the Bay.']]},
  {id:'plaza',name:'Outdoor Station',place:'San Leandro · station forecourt',asset:'scene-plaza.png',platform:2,pose:3,accent:'#b6a76e',spots:[['Cherry City',.08,.77,'San Leandro’s cherry-growing history inspired this little corner.'],['Bike parking',.19,.53,'Lock the bike. Save the beat. Catch the train.']]},
  {id:'tempo',name:'Transit / Bus Area',place:'San Leandro BART · Tempo 1T',asset:'scene-tempo.png',platform:2,pose:4,accent:'#ad79c7',spots:[['Tempo 1T',.74,.45,'Tempo Line 1T replaced the old Line 1. This stop connects San Leandro BART with Uptown Oakland.'],['Waiting for the 1',.14,.83,'“Still waiting for the 1…” — the sign says 1T now, Twon.']]},
  {id:'old-car',name:'Old School Car',place:'Legacy-car nostalgia',asset:'scene-old-car.png',platform:null,pose:5,accent:'#bc9f70',spots:[['Old school',.09,.35,'A nod to the original fleet. BART retired its legacy cars in April 2024.'],['Window seat',.19,.65,'A whole album on the way home.']]}
];
// Coordinates refer to the un-cropped 1586 × 992 scenery, not the window.
// Every route stays on one pavement/platform/aisle; no character crosses rails.
const FLOOR_PLANS={
 'platform-a':{twon:{x:.95,y:.94,size:.22},paths:[
  {id:'right-platform',points:[[.95,.99,.22],[.94,.84,.18],[.89,.70,.12],[.82,.55,.055]]},
  {id:'right-wall',points:[[1.04,.94,.23],[.97,.83,.17],[.94,.70,.11],[.88,.58,.062]]},
  {id:'opposite-platform',points:[[-.06,.61,.083],[.12,.58,.072],[.29,.55,.061],[.45,.515,.049]]}],seats:[]},
 'platform-b':{twon:{x:.95,y:.94,size:.22},paths:[
  {id:'right-platform',points:[[.93,.99,.22],[.92,.82,.17],[.88,.68,.10],[.825,.525,.049]]},
  {id:'right-wall',points:[[1.04,.93,.22],[.97,.80,.16],[.94,.67,.095],[.89,.55,.052]]},
  {id:'opposite-platform',points:[[-.06,.59,.085],[.12,.565,.076],[.31,.53,.061],[.48,.50,.045]]}],seats:[{id:'far-bench',x:.345,y:.489,size:.095}]},
 concourse:{twon:{x:.065,y:.632,size:.23,seated:true},paths:[
  {id:'near-forecourt',points:[[-.06,.96,.23],[.28,.94,.21],[.64,.94,.21],[1.06,.96,.23]]},
  {id:'station-walk',points:[[.17,.78,.16],[.38,.74,.145],[.66,.73,.14],[.88,.77,.155]]},
  {id:'entry-approach',points:[[.88,.96,.23],[.79,.84,.18],[.69,.74,.13],[.58,.66,.085]]}],seats:[{id:'entry-bench',x:.10,y:.635,size:.21}]},
 plaza:{twon:{x:.15,y:.692,size:.245,seated:true},paths:[
  {id:'front-path',points:[[-.06,.965,.25],[.30,.945,.23],[.65,.92,.215],[1.06,.90,.20]]},
  {id:'open-plaza',points:[[.31,.80,.17],[.51,.75,.145],[.73,.73,.135],[.92,.76,.15]]},
  {id:'station-approach',points:[[.82,.94,.235],[.75,.83,.18],[.67,.74,.13],[.60,.67,.095]]}],seats:[{id:'garden-bench',x:.21,y:.695,size:.22}]},
 tempo:{twon:{x:.13,y:.94,size:.24},paths:[
  {id:'shelter-front',points:[[-.06,.965,.24],[.29,.93,.215],[.63,.91,.21],[1.06,.94,.23]]},
  {id:'bus-waiting-area',points:[[.24,.79,.165],[.45,.75,.145],[.66,.755,.14],[.85,.79,.16]]},
  {id:'ticket-walk',points:[[.91,.96,.25],[.86,.85,.185],[.79,.76,.14],[.73,.71,.10]]}],seats:[{id:'shelter-bench',x:.065,y:.727,size:.24}]},
 'old-car':{twon:{x:.895,y:.922,size:.25,seated:true},paths:[
  {id:'center-aisle',points:[[.51,1.055,.255],[.51,.87,.20],[.49,.70,.13],[.49,.53,.062]]},
  {id:'aisle-left',points:[[.39,1.055,.25],[.43,.86,.19],[.46,.69,.115],[.49,.52,.060]]},
  {id:'aisle-right',points:[[.64,1.055,.25],[.60,.84,.185],[.55,.68,.115],[.51,.52,.059]]}],seats:[{id:'right-middle-seat',x:.607,y:.580,size:.13},{id:'left-middle-seat',x:.350,y:.690,size:.17},{id:'front-window-seat',x:.875,y:.925,size:.24}]}
};
const PEOPLE=Array.from({length:12},(_,i)=>i);
SCENES.forEach(s=>Object.assign(s,FLOOR_PLANS[s.id],{people:PEOPLE}));
const STORAGE_KEY='todb.environment.v1';
const COMMON=[['walk',42],['headphones',16],['phone',15],['wait',11],['bag',7],['sit',6],['sleep',3]];
const UNCOMMON=[['doubletake',4],['dance',3],['wave',2],['missed',1]];
const RARE=['bigdance','lasttrain','conversation'];
let settings={version:1,scene:'platform-a',animation:'reduced',midiGlow:true};
let root=null,dock=null,toast=null,world=null,npcLayer=null,twon=null,sign=null,sceneButton=null,sceneMenu=null,motionButtons=[],midiButton=null,exploreButton=null;
let initialized=false,exploring=false,onChange=()=>{},currentScene=SCENES[0],reducedMedia=null,sceneEpoch=0;
let raf=0,lastFrame=0,pulse=0,pulseTarget=0,drift=0,meterAt=0,lastNotes='',lastCharacter=-1,dimensions={w:1440,h:1000};
let actors=[],characterBag=[],seatedBag=[],recentBehaviors=[],pendingSettings=null,toastTimer=0,seatedReady=false;
const timers=new Map();
const rand=(a,b)=>a+Math.random()*(b-a);
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
const asset=name=>new URL(name,import.meta.url).href;
const motionMode=()=>reducedMedia?.matches?'off':settings.animation;
const motionEnabled=()=>initialized&&!document.hidden&&motionMode()!=='off';
const maxActors=()=>motionMode()==='reduced'?1:3;
const shuffle=a=>{const b=[...a];for(let i=b.length-1;i>0;i--){const j=Math.floor(Math.random()*(i+1));[b[i],b[j]]=[b[j],b[i]];}return b;};
const spriteFeet=[.932,.950,.952,.948,.966,.975,.962,.977,.977,.974,.978,.978];
function photoPoint(x,y){const scale=Math.max(dimensions.w/1586,dimensions.h/992);return{x:(dimensions.w-1586*scale)/2+x*1586*scale,y:(dimensions.h-992*scale)/2+y*992*scale,imageHeight:992*scale};}
function pathPoint(path,progress){const lengths=path.points.slice(1).map((p,i)=>Math.hypot((p[0]-path.points[i][0])*1.59879,p[1]-path.points[i][1]));const total=lengths.reduce((s,v)=>s+v,0);let along=clamp(progress,0,1)*total,at=0;while(at<lengths.length-1&&along>lengths[at])along-=lengths[at++];const a=path.points[at],b=path.points[at+1],t=lengths[at]?clamp(along/lengths[at],0,1):0;return{x:a[0]+(b[0]-a[0])*t,y:a[1]+(b[1]-a[1])*t,size:a[2]+(b[2]-a[2])*t,facing:b[0]<a[0]?-1:1};}
function weighted(items){const total=items.reduce((s,x)=>s+x[1],0);let roll=Math.random()*total;for(const [item,weight]of items){roll-=weight;if(roll<=0)return item;}return items[items.length-1][0];}
function normalized(input){const v=input&&typeof input==='object'?input:{};return{version:1,scene:SCENES.some(s=>s.id===v.scene)?v.scene:'platform-a',animation:['full','reduced','off'].includes(v.animation)?v.animation:'reduced',midiGlow:typeof v.midiGlow==='boolean'?v.midiGlow:true};}
function persist(){try{localStorage.setItem(STORAGE_KEY,JSON.stringify(settings));}catch{} }
function changed(){persist();onChange(environmentState());}
function schedule(name,ms,fn){clearTimer(name);if(!motionEnabled())return;timers.set(name,setTimeout(()=>{timers.delete(name);if(motionEnabled())fn();},ms));}
function clearTimer(name){if(timers.has(name)){clearTimeout(timers.get(name));timers.delete(name);}}
function clearMotion(){for(const timer of timers.values())clearTimeout(timer);timers.clear();if(raf)cancelAnimationFrame(raf);raf=0;lastFrame=0;actors.forEach(a=>a.el.remove());actors=[];pulse=pulseTarget=drift=0;root?.style.setProperty('--environment-pulse','0');if(root)root.dataset.npcs='0';}
function showToast(text){if(!toast)return;clearTimeout(toastTimer);toast.textContent=text;toast.hidden=false;toastTimer=setTimeout(()=>{toast.hidden=true;},6200);}
function makeButton(label,className=''){const b=document.createElement('button');b.type='button';b.textContent=label;b.className=className;return b;}
function blockSynthKeys(el){el.addEventListener('keydown',e=>e.stopPropagation());/* Key-up still bubbles so a note held before moving focus can release. */}

export function environmentState(){return{...settings};}
export function restoreEnvironment(state){if(!state)return;const next=normalized(state);if(!initialized){pendingSettings=next;settings=next;return;}const sceneChanged=next.scene!==settings.scene;settings=next;persist();if(sceneChanged)renderScene();else refreshMotion();updateDock();}
export function environmentMotion(){return{enabled:motionEnabled()&&settings.midiGlow,pulse:motionEnabled()&&settings.midiGlow?pulse:0,drift:motionEnabled()&&settings.midiGlow?drift:0};}
export function receiveEnvironmentMeter(m){
  if(!motionEnabled()||!settings.midiGlow)return;
  const notes=Array.isArray(m?.notes)?m.notes.filter(n=>Number.isFinite(n)&&n>=0&&n<=127):[];
  const identity=notes.join(',');
  const peak=clamp(Number(m?.peak)||0,0,1);
  const voices=clamp(Number(m?.voices)||0,0,32);
  const onset=identity!==lastNotes&&notes.length>0;
  lastNotes=identity;meterAt=performance.now();
  pulseTarget=clamp((notes.length||voices?Math.sqrt(peak)*.62+Math.min(notes.length,6)*.045:0)+(onset?.15:0),0,.75);
  if(pulseTarget>.01||pulse>.01)requestFrame();
}

export function initEnvironment({onChange:callback=()=>{}}={}){
  if(initialized){onChange=callback;return environmentState();}
  root=document.getElementById('environmentScene');dock=document.getElementById('environmentDock');toast=document.getElementById('environmentToast');
  if(!root||!dock||!toast)throw new Error('Environment roots are missing.');
  onChange=callback;reducedMedia=window.matchMedia('(prefers-reduced-motion: reduce)');
  if(pendingSettings)settings=pendingSettings;else{try{settings=normalized(JSON.parse(localStorage.getItem(STORAGE_KEY)||'{}'));}catch{settings=normalized({});}}
  initialized=true;document.body.classList.add('environment-active');
  root.className='environment-scene';root.setAttribute('aria-label','Illustrated Bay Area environment');root.setAttribute('role','region');root.dataset.npcs='0';
  world=document.createElement('div');world.className='environment-world';root.append(world);
  const backdrop=document.createElement('div');backdrop.className='environment-backdrop';backdrop.setAttribute('aria-hidden','true');world.append(backdrop);
  const light=document.createElement('div');light.className='environment-musical-light';light.setAttribute('aria-hidden','true');world.append(light);
  npcLayer=document.createElement('div');npcLayer.className='environment-commuters';npcLayer.setAttribute('aria-hidden','true');world.append(npcLayer);
  const label=document.createElement('div');label.className='environment-place';world.append(label);
  twon=makeButton('','environment-twon');twon.setAttribute('aria-label','Find Twon in the station');twon.title='Twon on da beat · a little station story';
  const twonArt=document.createElement('span');twonArt.className='environment-twon-art';twonArt.setAttribute('aria-hidden','true');twon.append(twonArt);world.append(twon);blockSynthKeys(twon);
  twon.onclick=()=>showToast(twonLine());twon.tabIndex=-1;
  sign=document.createElement('section');sign.className='environment-arrival';sign.setAttribute('aria-label','Simulated train information');world.append(sign);
  dock.className='environment-dock';dock.setAttribute('aria-label','Environment controls');blockSynthKeys(dock);
  const dockBrand=document.createElement('span');dockBrand.className='environment-dock-brand';dockBrand.innerHTML='<b>THE BAY</b><span>OUTSIDE THE CONSOLE</span>';dock.append(dockBrand);
  const previous=makeButton('‹','environment-arrow');previous.setAttribute('aria-label','Previous environment');previous.onclick=()=>switchRelative(-1);dock.append(previous);
  sceneButton=makeButton('','environment-scene-picker');sceneButton.setAttribute('aria-haspopup','listbox');sceneButton.setAttribute('aria-expanded','false');sceneButton.setAttribute('aria-controls','environmentSceneMenu');dock.append(sceneButton);
  sceneMenu=document.createElement('div');sceneMenu.id='environmentSceneMenu';sceneMenu.className='environment-scene-menu';sceneMenu.setAttribute('role','listbox');sceneMenu.setAttribute('aria-label','Choose environment');sceneMenu.hidden=true;
  SCENES.forEach(s=>{const b=makeButton(s.name);b.dataset.scene=s.id;b.setAttribute('role','option');b.setAttribute('aria-selected','false');b.onclick=()=>{chooseScene(s.id);closeMenu(true);};sceneMenu.append(b);});dock.append(sceneMenu);
  sceneButton.onclick=()=>sceneMenu.hidden?openMenu():closeMenu(true);
  sceneButton.onkeydown=e=>{if(['ArrowDown','ArrowUp','Home','End'].includes(e.key)){e.preventDefault();openMenu(e.key==='End'?SCENES.length-1:e.key==='Home'?0:SCENES.findIndex(s=>s.id===settings.scene));}else if(e.key==='Escape'){e.preventDefault();closeMenu(true);}};
  sceneMenu.onkeydown=e=>{const options=[...sceneMenu.querySelectorAll('button')],at=options.indexOf(document.activeElement);let next=at;if(e.key==='ArrowDown')next=(at+1)%options.length;else if(e.key==='ArrowUp')next=(at+options.length-1)%options.length;else if(e.key==='Home')next=0;else if(e.key==='End')next=options.length-1;else if(e.key==='Escape'){e.preventDefault();closeMenu(true);return;}else if(e.key==='Tab'){closeMenu(false);return;}else if(e.key.length===1){const target=options.findIndex(b=>b.textContent.toLowerCase().startsWith(e.key.toLowerCase()));if(target>=0)next=target;else return;}else return;e.preventDefault();options[next]?.focus();};
  const next=makeButton('›','environment-arrow');next.setAttribute('aria-label','Next environment');next.onclick=()=>switchRelative(1);dock.append(next);
  const group=document.createElement('div');group.className='environment-motion-group';group.setAttribute('role','radiogroup');group.setAttribute('aria-label','Background animation');
  const motionLabel=document.createElement('span');motionLabel.textContent='MOTION';motionLabel.className='environment-motion-label';group.append(motionLabel);
  motionButtons=['full','reduced','off'].map(mode=>{const b=makeButton(mode.toUpperCase());b.dataset.motion=mode;b.setAttribute('role','radio');b.onclick=()=>{settings.animation=mode;refreshMotion();updateDock();changed();};b.onkeydown=e=>{if(e.key==='ArrowLeft'||e.key==='ArrowRight'){e.preventDefault();const index=motionButtons.indexOf(b),other=motionButtons[(index+(e.key==='ArrowRight'?1:2))%3];other.click();other.focus();}};group.append(b);return b;});dock.append(group);
  midiButton=makeButton('MIDI lights','environment-midi-toggle');midiButton.onclick=()=>{settings.midiGlow=!settings.midiGlow;if(!settings.midiGlow){pulse=pulseTarget=drift=0;root.style.setProperty('--environment-pulse','0');}updateDock();changed();};dock.append(midiButton);
  exploreButton=makeButton('Explore station','environment-explore');exploreButton.onclick=()=>setExplore(!exploring);dock.append(exploreButton);
  toast.className='environment-toast';toast.setAttribute('role','status');toast.setAttribute('aria-live','polite');toast.hidden=true;blockSynthKeys(toast);
  document.addEventListener('pointerdown',e=>{if(!dock.contains(e.target))closeMenu(false);});
  document.addEventListener('keydown',e=>{if(exploring&&e.key==='Escape'&&!document.querySelector('dialog[open]')){e.preventDefault();e.stopPropagation();setExplore(false);exploreButton.focus();}},true);
  document.addEventListener('visibilitychange',refreshMotion);
  reducedMedia.addEventListener?.('change',()=>{refreshMotion();updateDock();});
  const resize=()=>{dimensions={w:Math.max(1,window.innerWidth),h:Math.max(1,window.innerHeight)};if(initialized&&twon&&twon.children.length&&sign)renderScene();};resize();window.addEventListener('resize',resize);
  renderScene();updateDock();return environmentState();
}

function setExplore(value){exploring=Boolean(value);document.body.classList.toggle('environment-exploring',exploring);exploreButton.textContent=exploring?'Return to synth':'Explore station';exploreButton.setAttribute('aria-pressed',String(exploring));twon.tabIndex=exploring?0:-1;world.querySelectorAll('.environment-hotspot').forEach(b=>b.tabIndex=exploring?0:-1);if(exploring)showToast('Look around. Your sound keeps playing. Escape returns to the synth.');}
function openMenu(index=SCENES.findIndex(s=>s.id===settings.scene)){sceneMenu.hidden=false;sceneButton.setAttribute('aria-expanded','true');sceneMenu.querySelectorAll('button')[Math.max(0,index)]?.focus();}
function closeMenu(focus){if(!sceneMenu||sceneMenu.hidden)return;sceneMenu.hidden=true;sceneButton.setAttribute('aria-expanded','false');if(focus)sceneButton.focus();}
function chooseScene(id){if(id===settings.scene)return;settings.scene=id;renderScene();updateDock();changed();}
function switchRelative(amount){const index=SCENES.findIndex(s=>s.id===settings.scene);chooseScene(SCENES[(index+amount+SCENES.length)%SCENES.length].id);}
function updateDock(){
  sceneButton.textContent=currentScene.name+' ▾';sceneButton.title=currentScene.place;
  sceneMenu.querySelectorAll('button').forEach(b=>b.setAttribute('aria-selected',String(b.dataset.scene===settings.scene)));
  motionButtons.forEach(b=>{const chosen=b.dataset.motion===settings.animation;b.setAttribute('aria-checked',String(chosen));b.tabIndex=chosen?0:-1;});
  midiButton.setAttribute('aria-pressed',String(settings.midiGlow));midiButton.title='Subtle visual response to notes and level. This never changes your sound.';
  dock.dataset.effectiveMotion=motionMode();dock.title=reducedMedia?.matches?'Background movement is off because Windows requests reduced motion.':'';
  exploreButton.setAttribute('aria-pressed',String(exploring));
}
function renderScene(){
  clearMotion();sceneEpoch++;currentScene=SCENES.find(s=>s.id===settings.scene)||SCENES[0];characterBag=[];recentBehaviors=[];
  root.dataset.scene=currentScene.id;root.style.setProperty('--environment-accent',currentScene.accent);
  world.querySelector('.environment-backdrop').style.backgroundImage=`url("${asset(currentScene.asset)}")`;
  world.querySelector('.environment-place').innerHTML=`<b>${currentScene.name}</b><span>${currentScene.place}</span>`;
  const tp=photoPoint(currentScene.twon.x,currentScene.twon.y),th=tp.imageHeight*currentScene.twon.size;twon.style.left=`${clamp(tp.x,th*.23+12,dimensions.w-th*.23-12)}px`;twon.style.top=`${tp.y}px`;twon.style.height=`${th}px`;twon.style.width=`${th*4/9}px`;
  const col=currentScene.pose%3,row=Math.floor(currentScene.pose/3);twon.querySelector('span').style.backgroundPosition=`${col*50}% ${row*100}%`;
  twon.setAttribute('aria-label',`Twon · ${currentScene.id==='tempo'?'waiting for the old 1 bus':'a station easter egg'}`);
  world.querySelectorAll('.environment-hotspot').forEach(el=>el.remove());
  currentScene.spots.forEach(([label,x,y,message])=>{const b=makeButton('+','environment-hotspot');b.style.left=`${x*100}%`;b.style.top=`${y*100}%`;b.setAttribute('aria-label',label);b.title=label;b.tabIndex=exploring?0:-1;b.onclick=()=>showToast(message);blockSynthKeys(b);world.append(b);});
  sign.hidden=currentScene.platform===null;sign.dataset.page='0';renderArrival(0);refreshMotion();
}
function twonLine(){const choices={
  'platform-a':['“Next stop: the hook.”','“Just one more take before the train.”'],
  'platform-b':['“The Bay runs on bass.”','“The release needs to be longer than my wait.”'],
  concourse:['“Don’t lose your Clipper. Don’t lose your session.”','“I made the beat before I made the train.”'],
  plaza:['“Cherry City. Purple session.”','“That breeze got swing.”'],
  tempo:['“Still waiting for the 1…” The bus is 1T now, Twon.','“Tempo outside. Tempo inside.”'],
  'old-car':['“These seats heard some classics.”','“Old school ride. New school beat.”']
};return choices[currentScene.id][Math.floor(Math.random()*2)];}
function renderArrival(page){
  if(currentScene.platform===null)return;
  const north=[['DALY CITY','8 CAR TRAIN','6 MIN'],['RICHMOND','6 CAR TRAIN','12 MIN']];
  const south=[['DUBLIN / PLEASANTON','8 CAR TRAIN','5 MIN'],['BERRYESSA / NORTH SAN JOSE','6 CAR TRAIN','14 MIN']];
  const pages=currentScene.platform===2?north:south;
  const content=pages[page%pages.length];
  sign.innerHTML=`<div class="environment-sign-top"><span>SAN LEANDRO</span><b>PLATFORM ${currentScene.platform}</b></div><div class="environment-led-destination">${content[0]}</div><div class="environment-led-detail"><span>${content[1]}</span><b>${content[2]}</b></div><small>DEMO · SIMULATED ARRIVALS</small>`;
  sign.dataset.page=String(page%pages.length);
}
function refreshMotion(){
  if(!initialized)return;clearMotion();lastNotes='';
  document.body.classList.toggle('environment-motion-off',motionMode()==='off');root.dataset.motion=motionMode();
  if(!motionEnabled())return;
  schedule('arrival',7500,cycleArrival);scheduleNextCommuter(true);scheduleNextRare();
}
function cycleArrival(){if(currentScene.platform!==null)renderArrival((Number(sign.dataset.page)||0)+1);schedule('arrival',7500,cycleArrival);}
function scheduleNextCommuter(first=false){const reduced=motionMode()==='reduced';schedule('commuter',rand(reduced?50000:first?18000:25000,reduced?120000:75000),()=>{if(actors.length<maxActors())spawnActor(false);scheduleNextCommuter();});}
function scheduleNextRare(){schedule('rare',rand(motionMode()==='reduced'?300000:120000,motionMode()==='reduced'?600000:300000),()=>{if(actors.length<maxActors())spawnActor(true);scheduleNextRare();});}
function nextCharacter(){if(!characterBag.length){characterBag=shuffle(currentScene.people);if(characterBag[characterBag.length-1]===lastCharacter){[characterBag[0],characterBag[characterBag.length-1]]=[characterBag[characterBag.length-1],characterBag[0]];}}const id=characterBag.pop();lastCharacter=id;return id;}
function nextBehavior(rare){let value;if(rare)value=RARE[Math.floor(Math.random()*RARE.length)];else value=Math.random()<.16?weighted(UNCOMMON):weighted(COMMON);if(recentBehaviors.slice(-2).every(x=>x===value)&&recentBehaviors.length>=2)value='walk';recentBehaviors.push(value);if(recentBehaviors.length>6)recentBehaviors.shift();return value;}
function spawnActor(rare){
  if(!motionEnabled()||actors.length>=maxActors())return;
  const character=nextCharacter(),behavior=nextBehavior(rare),path=currentScene.paths[Math.floor(Math.random()*currentScene.paths.length)];
  const reverse=Math.random()<.5,height=120,duration=rand(46000,92000)*(behavior==='missed'||behavior==='lasttrain'?.48:1);
  const el=document.createElement('div');el.className=`environment-person environment-behavior-${behavior}`;el.dataset.character=String(character);el.dataset.behavior=behavior;
  el.style.height=`${height}px`;el.style.width=`${height}px`;el.style.zIndex=String(10);
  const shadow=document.createElement('span');shadow.className='environment-person-shadow';el.append(shadow);
  const sprite=document.createElement('span');sprite.className='environment-person-sprite';sprite.style.backgroundPosition=`${character%4/3*100}% ${Math.floor(character/4)/2*100}%`;el.append(sprite);
  const needsPause=['phone','wait','headphones','dance','bigdance','doubletake','conversation','wave'].includes(behavior),pause=needsPause?rand(5000,14000):0;
  const phrase=behavior==='lasttrain'?'One more second!':behavior==='bigdance'?'This is my part.':behavior==='conversation'?'The remix, though…':null;
  if(phrase){const bubble=document.createElement('span');bubble.className='environment-character-bubble';bubble.textContent=phrase;el.append(bubble);}
  const seat=['sit','sleep'].includes(behavior)&&currentScene.seats.length?currentScene.seats[Math.floor(Math.random()*currentScene.seats.length)]:null;if(seat){const seated=character%8;sprite.style.backgroundImage=`url("${asset('npc-seated.png')}")`;sprite.style.backgroundSize='400% 200%';sprite.style.backgroundPosition=`${seated%4/3*100}% ${Math.floor(seated/4)*100}%`;}const actor={seat,el,sprite,start:performance.now(),duration,pause,pauseAt:rand(.26,.68),reverse,path,height,behavior,epoch:sceneEpoch};actors.push(actor);npcLayer.append(el);root.dataset.npcs=String(actors.length);requestFrame();
}
function requestFrame(){if(!raf&&motionEnabled())raf=requestAnimationFrame(frame);}
function frame(now){
  raf=0;if(!motionEnabled())return;
  if(lastFrame&&now-lastFrame<1000/30){requestFrame();return;}
  const dt=lastFrame?Math.min(.2,(now-lastFrame)/1000):1/30;lastFrame=now;
  if(now-meterAt>250)pulseTarget=0;pulse+=(pulseTarget-pulse)*Math.min(1,dt*7);drift+=(pulse*.008-drift)*Math.min(1,dt*1.8);
  root.style.setProperty('--environment-pulse',settings.midiGlow?pulse.toFixed(3):'0');
  actors=actors.filter(a=>{
    const elapsed=now-a.start,arrival=a.duration*a.pauseAt;
    let walked=elapsed,paused=false;if(a.pause&&elapsed>arrival){paused=elapsed<arrival+a.pause;walked=paused?arrival:elapsed-a.pause;}
    const progress=walked/a.duration;if(progress>=1||a.epoch!==sceneEpoch){a.el.remove();return false;}
    const pt=a.seat||pathPoint(a.path,a.reverse?1-progress:progress),pixel=photoPoint(pt.x,pt.y);const height=pixel.imageHeight*pt.size;a.el.style.height=`${height}px`;a.el.style.width=`${height}px`;const x=pixel.x-height/2,y=pixel.y-height*spriteFeet[Number(a.el.dataset.character)];
    const fade=Math.min(1,progress*14,(1-progress)*14);
    const dance=paused&&['dance','bigdance'].includes(a.behavior),bob=motionMode()==='reduced'?0:dance?Math.abs(Math.sin(elapsed/180))*3:paused?.3:Math.sin(elapsed/210)*1.1;
    const tilt=motionMode()==='reduced'?0:dance?Math.sin(elapsed/250)*5:paused&&a.behavior==='doubletake'?Math.sin(elapsed/700)*2:Math.sin(elapsed/420)*.5;
    a.el.style.transform=`translate3d(${x.toFixed(2)}px,${y.toFixed(2)}px,0)`;a.el.style.opacity=fade.toFixed(3);a.sprite.style.transform=`translateY(${-bob.toFixed(2)}px) rotate(${tilt.toFixed(2)}deg) scaleX(${a.reverse?-1:1})`;
    return true;
  });root.dataset.npcs=String(actors.length);
  if(actors.length||pulse>.005||pulseTarget>.005)requestFrame();else{lastFrame=0;root.style.setProperty('--environment-pulse','0');}
}
