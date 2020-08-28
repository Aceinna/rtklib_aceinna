function GUIMain_WriteListbox(str,hlistbox)

mStrs=get(hlistbox,'String');
len=length(mStrs);
mStrs{len+1}=str;
set(hlistbox,'String',mStrs);
set(hlistbox,'Value',len+1);
pause(0.01)