## usage
```bash
west build -t guiconfig
#west build -b decawave_dwm1001_dev -- -DCONF_FILE=prj.conf
west build -b qorvo_dwm3001cdk -- -DCONF_FILE=prj.conf
west flash
```
