function isok=generate_ini_batchproc(conf)
% genrate ini file for each obs data source and start batch processing
%SYNTAX:
%            ====================================
%            | isok=generate_ini_batchproc(conf)|
%            ====================================
%
%           
%IINPUT:
%
%
%  written by Yihe Li on 2019/03/19, Aceinna Inc
%  Note: 
%===========================BEGIN PROGRAM==============================%%
%open file for read only
org_ini='rtknavi.ini';
fp=fopen(org_ini,'r');
if(fp==-1)   error('error to open the '+ org_ini); end
num=length(conf);

for i=1:num
    inifile=strcat('rtknavi_b',num2str(i),'.ini');
    out(i).fp=fopen(inifile,'w'); 
end 

while (1) 
    strTemp0=fgets(fp);
    if(strTemp0==-1)  break; end
    for i=1:num
        
        if (length(strTemp0)>=8)
           if (strTemp0(1:8)=='path_0_1')
             fprintf(out(i).fp,'%s\n',strcat(strTemp0(1:9),conf(i).obspath)); 
          else
             fprintf(out(i).fp,'%s',strTemp0); 
          end 
        else
           fprintf(out(i).fp,'%s',strTemp0); 
        end 
        
    end 
end 

    fclose(fp);
    for i=1:num
       fclose(out(i).fp);
       copyfile('rtknavi.exe',strcat('rtknavi_b',num2str(i),'.exe'));
    end 
    
  for i=1:num
     command=strcat('rtknavi_b',num2str(i),'.exe -auto -tray &');
     status = system(command);
  end 
    
    isok=1;
  return;