"""Original synthesized UI feedback; no third-party audio samples."""
import math, random, struct, wave
from pathlib import Path
root=Path(__file__).parent/'Audio';root.mkdir(exist_ok=True)
for name,frequency,length in [('Body',700,.07),('Head',1450,.16),('Pickup',950,.20),('Hurt',140,.13)]:
    rng=random.Random(7); rate=48000; samples=[]
    for i in range(int(rate*length)):
        t=i/rate; e=math.sin(math.pi*min(1,t/.008)/2)*math.exp(-t/(length*.26))*min(1,(length-t)/.015)
        f=frequency*(1+(.35*t/length if name=='Pickup' else -.15*t/length))
        tone=math.sin(2*math.pi*f*t)+.25*math.sin(2*math.pi*f*1.5*t)
        if name in ['Body','Hurt']: tone+=.25*rng.uniform(-1,1)
        samples.append(int(max(-1,min(1,tone*.35*e))*32767))
    with wave.open(str(root/(name+'.wav')),'wb') as out:
        out.setnchannels(1);out.setsampwidth(2);out.setframerate(rate);out.writeframes(struct.pack('<'+'h'*len(samples),*samples))
