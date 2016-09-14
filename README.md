# microbit-simulated-dal
Simulated implementation of https://github.com/lancaster-university/microbit-dal/ for Grok

Initial setup:
```bash
# Clone our fork of bbcmicrobit/micropython and the simulated DAL.
cd ~/repos
git clone -b master-groksimulator git@github.com:groklearning/micropython.git bbcmicrobit-micropython
git clone -b master git@github.com:groklearning/microbit-simulated-dal.git

# Set up the local copy of simulated DAL and the build target as yotta targets.
# This allows you to work on your local copy and have the main build pick it up.
pushd microbit-simulated-dal
sudo yotta link
pushd targets/x86-linux-native-32bit
sudo yotta link-target
popd
git submodule init
git submodule update
popd

# Build for x86
cd bbcmicrobit-micropython
yotta link microbit-simulated-dal # Ignore warning
yotta link-target x86-linux-native-32bit # Ignore warning
yotta target x86-linux-native-32bit
yotta up
mkdir build
make inc/genhdr/qstrdefs.generated.h
yotta clean
yotta build
```

To copy the built simulator to marker/terminal:
```bash
sudo mkdir -p /markers/sandbox/opt/grok/devices/microbit
sudo chmod 0311 /markers/sandbox/opt/grok/devices/microbit
sudo cp build/x86-linux-native-32bit/source/microbit-micropython /markers/sandbox/opt/grok/devices/microbit
```

Development cycle:
```bash
yotta build
sudo cp build/x86-linux-native-32bit/source/microbit-micropython /markers/sandbox/opt/grok/devices/microbit
```

To pull changes from upstream:
```bash
git remote add upstream git@github.com:bbcmicrobit/micropython.git
git fetch upstream
git merge upstream/master
yotta clean
yotta build
```
