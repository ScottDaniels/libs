src = siaddres.c sibldpol.c sicbreg.c sicbstat.c siclose.c siconnec.c  \
	siestabl.c siexamp.c sigetadd.c sigetfd.c sigetname.c siinit.c silisten.c simcast.c \
	sinew.c sinewses.c sipoll.c sircv.c sireinit.c sisend.c sisendt.c sisendu.c sisetsig.c \
	sishutdo.c sisignal.c siterm.c sitest.c sitrash.c siwait.c  snagurl.c

broken = sidokey.c 

headers = sistruct.h siconst.h sisetup.h socket_if.h

libsi.a: $(src) $(headers)
	@gcc -c -I /usr/local/include $(src) 
	#gcc -I /usr/local/include siaddres.c -c
