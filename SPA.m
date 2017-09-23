clc;
if exist('s') == 1
    fclose(s);  
    delete(s);  
    clear s
    clear all;
    close all;
    clc;
end

s = serial('COM4');            %选择串口~select serial port
s.BaudRate = 115200;              %选择波特率 set baud rate
s.DataBits = 8;                 %设置数据位数 set data bits 
s.StopBits = 1;                 %设置停止位  set stop bit
set(s,'Parity', 'none','FlowControl','none');   %无校验位，无流控制 set parity and flow control
s.ReadAsyncMode = 'continuous';                 %异步接收模式为连续
s.BytesAvailableFcnMode = 'byte';               %回调函数模式为字节
exist_s = 0;
try
fopen(s);                               %打开串口
catch err
    fclose(s);  
    delete(s);  
    clear s
    clear all;
    close all;
    clc;
    fprintf('串口打开失败。\n');
end

%变量取值范围0~2： 0      0  
%                 2    65535
%设置参数
P0 = 1.3862;
I0 = 1.5216;

P1 = 1.2222;
I1 = 0.0001;

P2 = 0.5661;
I2 = 03879;

%数据返回标志
get_data = 0;

%将浮点数转换成给定通信格式的8位无符号整形变量
P01 = fix(P0*128);              %最大值为2，分辨率为16位=65536，取高8位，则为(P0/2*65535)/256 = P0*128,可根据所需要的精度对最大值与分辨率进行定制
P02 = fix(mod(P0*32768,256));   %取低8位，则为((P0*32768)%256)，下同
I01 = fix(I0*32768/256);
I02 = fix(mod(I0*32768,256));

P11 = fix(P1*128);
P12 = fix(mod(P1*32768,256));
I11 = fix(I1*128);
I12 = fix(mod(I1*32768,256));

P21 = fix(P2*128);
P22 = fix(mod(P2*32768,256));
I21 = fix(I2*128);
I22 = fix(mod(I2*32768,256));

%设定通信指令帧：02为设置P0，下同，发送完后延时等待下位机处理
inst = [170 171 02 P01 P02 255];
fwrite(s,inst,'uint8');
pause(0.02);
inst = [170 171 03 I01 I02 255];
fwrite(s,inst,'uint8');
pause(0.02);

inst = [170 171 04 P11 P12 255];
fwrite(s,inst,'uint8');
pause(0.02);
inst = [170 171 05 I11 I12 255];
fwrite(s,inst,'uint8');
pause(0.02);

inst = [170 171 06 P21 P22 255];
fwrite(s,inst,'uint8');
pause(0.02);
inst = [170 171 07 I21 I22 255];
fwrite(s,inst,'uint8');
pause(0.02);

%if exist_s == 0
    inst = [170 171 08 00 00 255];  %指令08用于读取返回下位机参数数值
    pause(0.1);
    while(get_data == 0)
        fwrite(s,inst,'uint8');
        pause(0.1);
        d = fread(s,[1 15],'uint8');
        if d(1) ==  170 && d(2) == 85   %判断下位机返回数据帧头
            P0_r = (d(3)*256+d(4))/32768
            I0_r = (d(5)*256+d(6))/32768
            P1_r = (d(7)*256+d(8))/32768
            I1_r = (d(9)*256+d(10))/32768
            P2_r = (d(11)*256+d(12))/32768
            I2_r = (d(13)*256+d(14))/32768
            get_data = 1;
        else
            d = [];
        end
    end
    fclose(s);      %关闭串口
    delete(s);
    clear s;
    clear all;
%end
