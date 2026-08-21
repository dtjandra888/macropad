
Had to navigate to pico sdk and use makefsdata on a fs directory with all html, css, js files

```
cd ~/repos/pico-sdk
find . -name "makefsdata"
```

cd to where it is, then copy over html, css, js files to fs in that dir

```
perl makefsdata
```

Should produce fsdata.c file. Copy this back over to here, and see output

I also had some issue with the webserver not being available. I ran these two commands to enable it

```
sudo ip addr add 192.168.7.2/24 dev <enx*>
sudo ip link set <enx*> up
```
