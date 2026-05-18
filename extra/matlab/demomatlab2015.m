function varargout = demomatlab2015(varargin)
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @demomatlab_OpeningFcn, ...
    'gui_OutputFcn',  @demomatlab_OutputFcn, ...
    'gui_LayoutFcn',  [], ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end

function demomatlab_OpeningFcn(hObject, ~, handles, varargin)
handles.output = hObject;
guidata(hObject, handles);
set(handles.demomat_gui,'ToolBar','none');
cla;
mex mexOgmacam.cpp -logmacam;
global bStop;
bStop=0;
[devN,devList]=mexOgmacam;
if (devN==0)
    set(handles.listbox_device,'string','No Device');
else
    setappdata(hObject,'Devices',devList);
    set(handles.listbox_device,'string',{devList.name});
end

function varargout = demomatlab_OutputFcn(~, ~, handles)
varargout{1} = handles.output;

function listbox_device_Callback(hObject, ~, handles) %#ok<DEFNU>
stype = get(gcf, 'SelectionType');
if ~strcmp(stype, 'open')
    return;
end
global bStop;
bStop=0;
set(hObject,'enable','off');
index=get(hObject,'value');
nResolutionIndex=0;
nSpeed=1;
devList=getappdata(gcf,'Devices');
[im, width, height, OgmacamData]=mexOgmacam(nResolutionIndex, nSpeed, devList(index).id, index);
setappdata(hObject,'OgmacamData',OgmacamData);
set(handles.slider_exposure_time,'enable','off');
set(handles.pushbutton_snap,'enable','on');
set(handles.pushbutton_awbOnePush,'enable','on');
set(handles.pushbutton_wbdefaults,'enable','on');
set(handles.pushbutton_defaults,'enable','on');
set(handles.slider_temp,'Enable','on');
set(handles.slider_temp,'Value',OgmacamData.OGMACAM_TEMP_DEF);
set(handles.slider_tint,'Enable','on');
set(handles.slider_tint,'Value',OgmacamData.OGMACAM_TINT_DEF);
set(handles.text_temp,'Visible','on');
set(handles.text_temp,'String',num2str(OgmacamData.OGMACAM_TEMP_DEF));
set(handles.text_tint,'Visible','on');
set(handles.text_tint,'String',num2str(OgmacamData.OGMACAM_TINT_DEF));
set(handles.text_hue,'Visible','on');
set(handles.text_hue,'String',num2str(OgmacamData.OGMACAM_HUE_DEF));
set(handles.slider_hue,'Enable','on');
set(handles.slider_hue,'Value',OgmacamData.OGMACAM_HUE_DEF);
set(handles.text_saturation,'Visible','on');
set(handles.text_saturation,'String',num2str(OgmacamData.OGMACAM_SATURATION_DEF));
set(handles.slider_saturation,'Enable','on');
set(handles.slider_saturation,'Value',OgmacamData.OGMACAM_SATURATION_DEF);
set(handles.text_brightness,'Visible','on');
set(handles.text_brightness,'String',num2str(OgmacamData.OGMACAM_BRIGHTNESS_DEF));
set(handles.slider_brightness,'Enable','on');
set(handles.slider_brightness,'Value',OgmacamData.OGMACAM_BRIGHTNESS_DEF);
set(handles.text_contrast,'Visible','on');
set(handles.text_contrast,'String',num2str(OgmacamData.OGMACAM_CONTRAST_DEF));
set(handles.slider_contrast,'Enable','on');
set(handles.slider_contrast,'Value',OgmacamData.OGMACAM_CONTRAST_DEF);
set(handles.text_gamma,'Visible','on');
set(handles.text_gamma,'String',num2str(OgmacamData.OGMACAM_GAMMA_DEF));
set(handles.slider_gamma,'Enable','on');
set(handles.slider_gamma,'Value',OgmacamData.OGMACAM_GAMMA_DEF);
set(handles.checkbox_auto_exposure, 'Enable', 'on')
img=zeros(height,width, 3);
warning('off'); 
axes(handles.axes_vedio);
while ~isequal(bStop,2)
    for i=1:height
        for j=1:width
            img(i,j,1)=im(3*(j-1)+3,i);
            img(i,j,2)=im(3*(j-1)+2,i);
            img(i,j,3)=im(3*(j-1)+1,i);
        end
    end
    imshow(uint8(img));
    drawnow;
    if bStop==1
        uiwait;
    end
end
mexOgmacam(0,0);
clear im;

function listbox_device_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function pushbutton_snap_Callback(~, ~, ~) %#ok<DEFNU>
mexOgmacam(1,0);

function checkbox_auto_exposure_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1; %#ok<*NASGU>
[a,time]=mexOgmacam(2,0);
time=roundn(time/1000,-2);
if (a==0)
    set(hObject,'Value',0);
    set(handles.slider_exposure_time,'enable','on');
    set(handles.text_exposure_time,'Visible','on');
    set(handles.slider_exposure_time,'Value',time);
    set(handles.text_exposure_time,'String',num2str(time));
else
    set(hObject,'Value',1);
    set(handles.slider_exposure_time,'enable','off');
    set(handles.text_exposure_time,'Visible','off');
end
bStop = 0;
uiresume;

function checkbox_auto_exposure_CreateFcn(~, ~, ~) %#ok<DEFNU>

function slider_exposure_time_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=get(hObject,'Value');
set(handles.text_exposure_time,'Visible','on');
set(handles.text_exposure_time,'String',num2str(roundn(val,-2)));
mexOgmacam(3,val*1000);
bStop = 0;
uiresume;

function slider_exposure_time_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function pushbutton_awbOnePush_Callback(~, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
[temp,tint]=mexOgmacam(4,0);
set(handles.text_temp,'String',num2str(temp));
set(handles.text_tint,'String',num2str(tint));
set(handles.slider_temp,'Value',temp);
set(handles.slider_tint,'Value',tint);
bStop = 0;
uiresume;

function slider_temp_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_temp,'String',num2str(val));
mexOgmacam(5,val);
bStop = 0;
uiresume;

function slider_temp_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function slider_tint_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_tint,'String',num2str(val));
mexOgmacam(6,val);
bStop = 0;
uiresume;

function slider_tint_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function pushbutton_wbdefaults_Callback(~, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
mexOgmacam(12,0);
OgmacamData = getappdata(handles.listbox_device,'OgmacamData');
set(handles.text_temp,'String', num2str(OgmacamData.OGMACAM_TEMP_DEF));
set(handles.slider_temp,'Value', OgmacamData.OGMACAM_TEMP_DEF);
set(handles.text_tint,'String', num2str(OgmacamData.OGMACAM_TINT_DEF));
set(handles.slider_tint,'Value', OgmacamData.OGMACAM_TINT_DEF);
bStop = 0;
uiresume;

function slider_hue_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_hue,'String',num2str(val));
mexOgmacam(7,val);
bStop = 0;
uiresume;

function slider_hue_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function slider_saturation_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_saturation,'String',num2str(val));
mexOgmacam(8,val);
bStop = 0;
uiresume;

function slider_saturation_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function slider_brightness_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_brightness,'String',num2str(val));
mexOgmacam(9,val);
bStop = 0;
uiresume;

function slider_brightness_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function slider_contrast_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_contrast,'String',num2str(val));
mexOgmacam(10,val);
bStop = 0;
uiresume;

function slider_contrast_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function slider_gamma_Callback(hObject, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
val=round(get(hObject,'Value'));
set(handles.text_gamma,'String',num2str(val));
mexOgmacam(11,val);
bStop = 0;
uiresume;

function slider_gamma_CreateFcn(hObject, ~, ~) %#ok<DEFNU>
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

function pushbutton_defaults_Callback(~, ~, handles) %#ok<DEFNU>
global bStop;
bStop = 1;
OgmacamData = getappdata(handles.listbox_device,'OgmacamData');
mexOgmacam(13,0);
set(handles.text_hue,'String', num2str(OgmacamData.OGMACAM_HUE_DEF));
set(handles.slider_hue,'Value', OgmacamData.OGMACAM_HUE_DEF);
set(handles.text_saturation,'String', num2str(OgmacamData.OGMACAM_SATURATION_DEF));
set(handles.slider_saturation,'Value', OgmacamData.OGMACAM_SATURATION_DEF);
set(handles.text_brightness,'String', num2str(OgmacamData.OGMACAM_BRIGHTNESS_DEF));
set(handles.slider_brightness,'Value', OgmacamData.OGMACAM_BRIGHTNESS_DEF);
set(handles.text_contrast,'String', num2str(OgmacamData.OGMACAM_CONTRAST_DEF));
set(handles.slider_contrast,'Value', OgmacamData.OGMACAM_CONTRAST_DEF);
set(handles.text_gamma,'String', num2str(OgmacamData.OGMACAM_GAMMA_DEF));
set(handles.slider_gamma,'Value', OgmacamData.OGMACAM_GAMMA_DEF);
bStop = 0;
uiresume;

function demomat_gui_CloseRequestFcn(hObject, ~, ~) %#ok<DEFNU>
global bStop;
bStop = 2;
delete(hObject);
