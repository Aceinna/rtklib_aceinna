function conf=read_configure(filename)
% read configure file and prepare for batch processing
% SYNTAX:
%            ==================================
%            | conf=read_configure(filename)|
%            =================================
%
%           
%INPUT:
%   filename: configure file name (ppp.conf)
%
%OUTPUT:
%   conf: confifure information, the input parameters can be edited
%   for each obs data (station). currently, only following parameters are
%   supported.
%   other parameters can be edited in rtknavi.ini, and applied to all
%   stations
%   obspath: obs data source
%   ssrpath: ssr correction source
%   brdcpath: brdc source
%  written by Yihe Li on 2019/03/19, Aceinna Inc
%  Note: 
%===========================BEGIN PROGRAM==============================%%
if(nargin<1)
    error('not enough input argument');
end

%open file for read only
fp=fopen(filename,'r');
if(fp==-1)   error('error to open the '+ filename); end

strTemp=fgets(fp);
strTemp=fgets(fp);
stanum=str2num(strTemp(21:22));

stacount=0;
while (1) 
    strTemp=fgets(fp);
    if(strTemp==-1) 
        break;
    end

    if(strTemp(1:2)=='$$')
        
        stacount=stacount+1;
        
        num=str2num(strTemp(12:13));
        for i=1:num
            
            strTemp=fgets(fp);
            
            if (strTemp(1:12)=='inpstr1-path')
              conf(stacount).obspath=strTemp(21:end);
            end 
            
            if (strTemp(1:12)=='inpstr2-path')
              conf(stacount).ssrpath=strTemp(21:end);
            end 
            
            if (strTemp(1:12)=='inpstr3-path')
              conf(stacount).brdcpath=strTemp(21:end);
            end 
        end 
        
    end 
    
end 

return