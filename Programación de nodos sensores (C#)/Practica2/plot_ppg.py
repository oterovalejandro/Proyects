#!/usr/bin/python
import sys, numpy, pylab

### Check args
if len(sys.argv) < 3 :
    sys.stderr.write("Usage: plot_photoplet.py <file> <freq>\n")
    sys.exit(1)

### Load file
# binary file
x = numpy.memmap(sys.argv[1], dtype='h', mode='r')
print("read ", len(x), " samples from file ", sys.argv[1])
# text file
#x = numpy.loadtxt(sys.argv[1], dtype="int", delimiter=" ")
x = x * 3.3/1024.0

x = x-min(x) 
x = x/max(x)*100.0
#mu = numpy.mean(x)
#x= x -mu
n = len(x)

### Time related vars
Fs = 1000.0 
k = numpy.arange(n)
T = n/Fs
frq = k/T
frq = frq[range(n//2)]
Ts = 1.0/Fs; 
t = numpy.linspace(0,n*Ts, n) #numpy.arange(0,10,Ts)

### FFT
fft = numpy.fft.rfft(x)
X = fft/n
X = X[range(n//2)]

print("len(fft)=",len(fft), " len(X)=" , len(X))


### Lowpass filter
cutoff = float(sys.argv[2])
f = numpy.linspace(0.0, 100.0, n//2+1) #// == integer devision
H = numpy.where(f<cutoff, 1.0, 0.0)
x_f = numpy.fft.irfft(fft*H)

print("len(f)=",len(f),"len(H)=",len(H),"len(x_f)",len(x_f))

### Plot commands
pylab.subplot(3,1,1)
pylab.plot(t,x)
pylab.title("RAW data - file =" + sys.argv[1])
#pylab.xlabel("Time Elapsed (seconds)")
pylab.ylabel("Amplitude (% max)")
pylab.grid()
pylab.subplot(3,1,2)
pylab.plot(frq,abs(X))
pylab.title("Frequency analysis")
#pylab.xlabel("Freq (Hz)")
pylab.ylabel("|X(freq)|")
pylab.xlim([0,100])
pylab.grid()
pylab.subplot(3,1,3)
pylab.plot(t,x_f)
pylab.title("Filtered signal wth lowpass=" + sys.argv[2] + " Hz" )
#pylab.xlabel("Time Elapsed (seconds)")
pylab.ylabel("Amplitude (% max)")
pylab.grid()
pylab.tight_layout()
pylab.show()


