% MATLAB script to generate sample 'input.json'
% for demonstration
%
% 2018.3 Ryotaro Sato

clear
close all
rng(0)

fs_Hz = 125;
sigLen = 400;
timeSeq = (1:sigLen) / fs_Hz;

up = zeros(1, sigLen);
up(20) = 0.8 * fs_Hz;

ua = zeros(1, sigLen);
ua(50:100) = 0.2;
ua(200:320) = 0.6;

mub = 60.0;

vuv = zeros(1, sigLen);
vuv(30:150) = 1;
vuv(210:250) = 1;
vuv(300:360) = 1;

xp = fujisakiFilter(up, fs_Hz, 3.0);
xa = fujisakiFilter(ua + randn(1, sigLen) / 5, fs_Hz, 20.0);
logF0 = xp + xa + log(mub);

inputData.fs = fs_Hz;
inputData.initial_mub = log(mub);
inputData.initial_ua = ua;
inputData.initial_up = up;
inputData.logf0 = logF0;
inputData.vuv = vuv;

jsonData = jsonencode(inputData);

fid = fopen('input.json', 'w');
fprintf(fid, jsonData);
fclose(fid);

figure()
subplot(2, 1, 1)
plot(timeSeq, up / fs_Hz)
hold on
plot(timeSeq, ua)
ylabel('Amplitude')

subplot(2, 1, 2)
plot(timeSeq, logF0 .* vuv)
xlabel('Time [s]')
ylabel('logF0')


function result = fujisakiFilter( commandSeq, fs, omega )
timeSeq = (0:(length(commandSeq)-1)) / fs;
impulseResponse = omega * omega .* timeSeq .* exp(-omega * timeSeq) / fs;
result = conv(commandSeq, impulseResponse);
result = result(1:length(commandSeq));
end
