## Yotta Target Description for x86 32bit on Linux, using Native Runtimes

Use this target to build things using the native compiler and runtimes.

This target can only be used when the host system is running Linux.

```bash
# In this directory
yotta link-target
```

```bash
# In module directory
yotta target x86-linux-native-32bit
yotta link-target x86-linux-native-32bit
...
yotta install
```