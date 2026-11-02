// Schematic coordinates; station names and connectivity based on bart.gov/system-map (2026-10-03).
export const stations=[];
function branch(rows){return rows.map(([id,name,x,y,param])=>{stations.push({id,name,x,y,param});return id})}
const north=branch([['rich','Richmond',485,55,'pitch'],['deln','El Cerrito del Norte',485,87,'mix'],['plza','El Cerrito Plaza',485,119,'detune'],['nbrk','North Berkeley',485,151,'fm'],['dbrk','Downtown Berkeley',485,183,'morph'],['ashb','Ashby',485,215,'noise'],['mcar','MacArthur',485,267,'cutoff']]);
const east=branch([['antc','Antioch',910,68,'arp'],['pctr','Pittsburg Center',866,91,'probability'],['pitt','Pittsburg / Bay Point',821,114,'swing'],['ncon','North Concord / Martinez',775,137,'lfo2Rate'],['conc','Concord',731,160,'lfo2Depth'],['phil','Pleasant Hill / Contra Costa Centre',687,183,'modAttack'],['wcrk','Walnut Creek',643,206,'modDecay'],['lafy','Lafayette',600,229,'modSustain'],['orin','Orinda',557,252,'modRelease'],['rock','Rockridge',525,267,'keytrack']]);
const oak=branch([['19th','19th St. Oakland',485,312,'resonance'],['12th','12th St. Oakland City Center',485,345,'filterEnv'],['lake','Lake Merritt',551,393,'filterAttack'],['ftvl','Fruitvale',581,428,'ampDecay'],['cols','Coliseum',611,463,'ampSustain'],['sanl','San Leandro',641,498,'glide'],['bayf','Bay Fair',671,533,'gate']]);
const south=branch([['hayw','Hayward',671,573,'ampRelease'],['shay','South Hayward',671,605,'sub'],['ucty','Union City',671,637,'lfo3Rate'],['frmt','Fremont',671,669,'lfo3Depth'],['warm','Warm Springs / South Fremont',671,701,'octave3'],['mlpt','Milpitas',671,733,'width'],['bery','Berryessa / North San Jose',671,765,'master']]);
const dublin=branch([['cast','Castro Valley',731,533,'pitchEnv'],['wdub','West Dublin / Pleasanton',810,533,'pitchDecay'],['dubl','Dublin / Pleasanton',900,533,'material']]);
branch([['woak','West Oakland',420,367,'drive']]);
const sf=branch([['embr','Embarcadero',301,406,'delayMix'],['mont','Montgomery St.',279,433,'feedback'],['powl','Powell St.',257,460,'reverb'],['civc','Civic Center / UN Plaza',235,487,'chorus'],['16th','16th St. Mission',213,514,'phaser'],['24th','24th St. Mission',191,541,'flanger'],['glen','Glen Park',170,568,'lfo1Rate'],['balb','Balboa Park',151,595,'lfo1Depth'],['daly','Daly City',132,622,'vibrato']]);
const peninsula=branch([['colm','Colma',132,654,'glideMode'],['ssan','South San Francisco',132,686,'glideCurve'],['sbrn','San Bruno',132,718,'bendRange']]);
branch([['sfia','San Francisco International Airport',235,743,'compressor'],['mlbr','Millbrae',132,767,'eq'],['oakl','Oakland International Airport',519,503,'bitcrusher']]);
const spine=['mcar','19th','12th','woak',...sf];const eastSpine=[...east,'mcar'];
export const routes=[
 {id:'yellow',name:'Yellow · Antioch → SFO',color:'#ffe063',stops:[...eastSpine,...spine.slice(1),...peninsula,'sfia']},
 {id:'red',name:'Red · Richmond → Millbrae',color:'#ff666c',stops:[...north,...spine.slice(1),...peninsula,'sfia','mlbr']},
 {id:'blue',name:'Blue · Dublin → Daly City',color:'#6aa9ff',stops:[...dublin].reverse().concat(['bayf','sanl','cols','ftvl','lake','woak'],sf)},
 {id:'green',name:'Green · Berryessa → Daly City',color:'#65d699',stops:[...south].reverse().concat(['bayf','sanl','cols','ftvl','lake','woak'],sf)},
 {id:'orange',name:'Orange · Richmond → Berryessa',color:'#ffa963',stops:[...north,...oak,...south]},
 {id:'airport',name:'Airport · Coliseum → OAK',color:'#c5b6a0',stops:['cols','oakl']}
];
export const byId=Object.fromEntries(stations.map(s=>[s.id,s]));

// Layout traced from the user's supplied daytime-service map. Station order is
// stable so project route-note data continues to recall correctly.
const mapCoordinates={rich:[200,73],deln:[218,105],plza:[237,124],nbrk:[256,143],dbrk:[275,162],ashb:[295,181],mcar:[310,207],antc:[605,62],pctr:[545,62],pitt:[515,62],ncon:[467,62],conc:[435,93],phil:[416,110],wcrk:[398,127],lafy:[379,146],orin:[359,165],rock:[338,184],'19th':[310,225],'12th':[310,245],lake:[327,282],ftvl:[346,300],cols:[365,322],sanl:[386,345],bayf:[408,368],hayw:[445,403],shay:[472,430],ucty:[499,456],frmt:[526,484],warm:[542,526],mlpt:[542,553],bery:[542,582],cast:[469,382],wdub:[545,382],dubl:[611,382],woak:[253,267],embr:[222,272],mont:[209,286],powl:[193,302],civc:[177,319],'16th':[165,355],'24th':[165,374],glen:[165,392],balb:[165,410],daly:[185,444],colm:[203,470],ssan:[224,491],sbrn:[246,513],sfia:[250,521],mlbr:[264,545],oakl:[350,392]};
for(const s of stations){[s.x,s.y]=mapCoordinates[s.id];}
const geometry=new Map();
export function routeGeometry(route){
 if(geometry.has(route.id))return geometry.get(route.id);
 const pts=[];let previous;
 for(const id of route.stops){const s=byId[id];const pair=[previous,id].sort().join('/');
  if(pair==='12th/woak')pts.push({x:310,y:267});
  if(pair==='lake/woak')pts.push({x:310,y:267});
  if(pair==='bayf/cast')pts.push({x:423,y:382});
  if(pair==='cols/oakl')pts.push({x:364,y:365},{x:350,y:380});
  if(pair==='civc/16th'.split('/').sort().join('/'))pts.push({x:165,y:337});
  pts.push({x:s.x,y:s.y});previous=id;
 }
 const lane=[-5,-10,0,5,10,0][routes.indexOf(route)];
 const shifted=pts.map((p,i)=>{let a=pts[Math.max(0,i-1)],b=pts[Math.min(pts.length-1,i+1)],dx=b.x-a.x,dy=b.y-a.y,len=Math.hypot(dx,dy)||1;return{x:p.x-dy/len*lane,y:p.y+dx/len*lane}});
 // Rounded bends share the identical sampled points used by the trains.
 const rounded=[shifted[0]];
 for(let i=1;i<shifted.length-1;i++){let a=shifted[i-1],b=shifted[i],c=shifted[i+1],ab=Math.hypot(b.x-a.x,b.y-a.y),bc=Math.hypot(c.x-b.x,c.y-b.y),d=Math.min(9,ab*.3,bc*.3),q={x:b.x+(a.x-b.x)*d/ab,y:b.y+(a.y-b.y)*d/ab},r={x:b.x+(c.x-b.x)*d/bc,y:b.y+(c.y-b.y)*d/bc};rounded.push(q);for(let j=1;j<=12;j++){let t=j/12;rounded.push({x:(1-t)**2*q.x+2*(1-t)*t*b.x+t*t*r.x,y:(1-t)**2*q.y+2*(1-t)*t*b.y+t*t*r.y})}}
 rounded.push(shifted.at(-1));let total=0;const lengths=rounded.slice(1).map((p,i)=>{let l=Math.hypot(p.x-rounded[i].x,p.y-rounded[i].y);total+=l;return l});
 const data={points:rounded,lengths,total};geometry.set(route.id,data);return data;
}
export function pointOnRoute(route,position){const{points,lengths,total}=routeGeometry(route);let d=Math.max(0,Math.min(1,position))*total;for(let i=0;i<lengths.length;i++){if(d<=lengths[i]||i===lengths.length-1){const t=d/(lengths[i]||1),a=points[i],b=points[i+1];return{x:a.x+(b.x-a.x)*t,y:a.y+(b.y-a.y)*t,angle:Math.atan2(b.y-a.y,b.x-a.x)*180/Math.PI,index:i,fraction:t}}d-=lengths[i]}}
