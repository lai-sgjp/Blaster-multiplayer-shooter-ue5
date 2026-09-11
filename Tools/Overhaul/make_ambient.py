"""Original low-volume wind bed, periodic Fourier noise; no sampled audio."""
import math,random,struct,wave
from pathlib import Path
rate=24000;length=8;n=rate*length;rng=random.Random(23)
partials=[(rng.randint(70*length,550*length),rng.uniform(0,math.tau),rng.uniform(.1,1)) for _ in range(48)]
data=[]
for i in range(n):
 t=i/n
 noise=sum(math.sin(math.tau*f*t+p)*a for f,p,a in partials)/48
 data.append(int(noise*.14*(.7+.3*math.sin(math.tau*t))*min(1,i/1200,(n-1-i)/1200)*32767))
with wave.open(str(Path(__file__).parent/'Audio'/'StreetWind.wav'),'wb') as f:
 f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes(struct.pack('<'+'h'*n,*data))
