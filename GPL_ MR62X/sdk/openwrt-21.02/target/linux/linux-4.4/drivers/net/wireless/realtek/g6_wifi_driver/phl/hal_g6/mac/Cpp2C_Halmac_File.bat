set LOCAL_FOLDER_ADDR=%~DP0%

copy	%LOCAL_FOLDER_ADDR%*.cpp %LOCAL_FOLDER_ADDR%*.c
copy	%LOCAL_FOLDER_ADDR%chip_cfg.h %LOCAL_FOLDER_ADDR%chip_cfg_tmp.h
copy	%LOCAL_FOLDER_ADDR%feature_cfg.h %LOCAL_FOLDER_ADDR%feature_cfg_tmp.h
copy	%LOCAL_FOLDER_ADDR%pltfm_cfg.h %LOCAL_FOLDER_ADDR%pltfm_cfg_tmp.h
copy	%LOCAL_FOLDER_ADDR%chip_cfg_drv.h %LOCAL_FOLDER_ADDR%chip_cfg.h
copy	%LOCAL_FOLDER_ADDR%feature_cfg_drv.h %LOCAL_FOLDER_ADDR%feature_cfg.h
copy	%LOCAL_FOLDER_ADDR%pltfm_cfg_drv.h %LOCAL_FOLDER_ADDR%pltfm_cfg.h
del		%LOCAL_FOLDER_ADDR%chip_cfg_drv.h 
del		%LOCAL_FOLDER_ADDR%feature_cfg_drv.h
del		%LOCAL_FOLDER_ADDR%pltfm_cfg_drv.h 
del		%LOCAL_FOLDER_ADDR%*.cpp

copy	%LOCAL_FOLDER_ADDR%mac_ax\*.cpp %LOCAL_FOLDER_ADDR%mac_ax\*.c
del		%LOCAL_FOLDER_ADDR%mac_ax\*.cpp 

copy	%LOCAL_FOLDER_ADDR%mac_ax\mac_8852a\*.cpp %LOCAL_FOLDER_ADDR%mac_ax\mac_8852a\*.c
copy	%LOCAL_FOLDER_ADDR%mac_ax\mac_8852b\*.cpp %LOCAL_FOLDER_ADDR%mac_ax\mac_8852b\*.c
del		%LOCAL_FOLDER_ADDR%mac_ax\mac_8852a\*.cpp
del		%LOCAL_FOLDER_ADDR%mac_ax\mac_8852b\*.cpp

rd/s/q		%LOCAL_FOLDER_ADDR%hv_ax

copy	%LOCAL_FOLDER_ADDR%doc\mac.* %LOCAL_FOLDER_ADDR%
copy	%LOCAL_FOLDER_ADDR%doc\mac_8852a.* %LOCAL_FOLDER_ADDR%mac_ax\mac_8852a
copy	%LOCAL_FOLDER_ADDR%doc\mac_8852b.* %LOCAL_FOLDER_ADDR%mac_ax\mac_8852b
rd/s/q	%LOCAL_FOLDER_ADDR%doc