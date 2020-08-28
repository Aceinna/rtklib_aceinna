function isok=regression(proc_time)

% main function to do regression test (batch processing) for real-time PPP
% SYNTAX:
%             ======================
%                || regression  ||
%             ======================
%
%INPUT:
%   proc_time: regresion proceeding time
%
%  written by Yihe Li on 2019/03/19, Aceinna Inc
%  Note: 
%%============================BEGIN PROGRAM==========================%%

title=sprintf('Real-time PPP Bacth Processing....');
set(gcf,'MenuBar','none','NumberTitle','off','Name',title,...
    'Position',[400 500 450 150],'Visible','off');
movegui(gcf,'center');
hlistbox = uicontrol(gcf,'Style', 'listbox', 'String', 'Clear',...
    'Position', [0 0 450 150],'String','','Foregroundcolor','b','Backgroundcolor','w');
set(gcf,'Visible','on');
movegui(gcf,'center');
warning('off');

conffile='ppp.conf';

tStart=tic;
    
GUIMain_WriteListbox('Step 1: Reading configure files ......',hlistbox);

conf=read_configure(conffile);

GUIMain_WriteListbox('Step 2: Processing ......',hlistbox);

isok=generate_ini_batchproc(conf);

while(1)
    if (toc(tStart)>proc_time)
      for i=1:length(conf)
         inifile = strcat('rtknavi_b',num2str(i),'.ini');
         proc    = strcat('rtknavi_b',num2str(i),'.exe');
         command=strcat('taskkill /f /im',32,proc);
         status = system(command);
         delete(proc);
         delete(inifile);
      end 
      break;
    end 
end 

str=strcat('Finished : converged time ',32,num2str(toc(tStart)/60),' minutes');
GUIMain_WriteListbox(str,hlistbox);
close (gcf)

fclose all;
for i=1:length(conf)
    inifile = strcat('rtknavi_b',num2str(i),'.ini');
    proc    = strcat('rtknavi_b',num2str(i),'.exe');
    delete(inifile);
    delete(proc);
end 

return;
















    
    



    