
srcFiles = [
    "siaddres.c",
    "sisendt.c",
]

xsrcFiles = [
    "sibldpol.c",
    "sicbreg.c",
    "sicbstat.c",
    "siclose.c",
    "siconnec.c",
    "siconst.h",
    "sidokey.c",
    "siestabl.c",
    "siexamp.c",
    "sigetadd.c",
    "sigetfd.c",
    "sigetname.c",
    "siinit.c",
    "silisten.c",
    "simcast.c",
    "sinew.c",
    "sinewses.c",
    "sipoll.c",
    "sircv.c",
    "sireinit.c",
    "sisend.c",
    "sisendt.c",
    "sisendu.c",
    "sisetsig.c",
    "sishutdo.c",
    "sisignal.c",
    "siterm.c",
    "sitest.c",
    "sitrash.c",
    "siwait.c",
    "snagurl.c",
]

# these are visible to rules which depend on the libary
pubHeaders = [
    "socket_if.h", 
]

# these are not exported
headers = [ 
    "sisetup.h",
    "siconst.h",
    "sistruct.h",
]

cc_library(
    name = "libsi",
    srcs = srcFiles, 
    hdrs = pubHeaders,
    private_hdrs = headers,
    #out = defaults to libsi.a
    visibility = [ "PUBLIC" ],
)
