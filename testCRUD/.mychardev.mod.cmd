savedcmd_/home/johnson/test/mychardev.mod := printf '%s\n'   mychardev.o | awk '!x[$$0]++ { print("/home/johnson/test/"$$0) }' > /home/johnson/test/mychardev.mod
