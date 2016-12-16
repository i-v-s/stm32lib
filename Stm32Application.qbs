import qbs

Product {
    Depends { name : "cpp" }
    property string deviceName: "none"

    property variant device: {
        // pin, mem, pack
        var dn = deviceName.replace(/^stm32f(\d\d\d)(\S)(\S)\S*$/i, 'STM32F$1$2$3Tx').toUpperCase();
        var family = deviceName.replace(/^stm32f(\d)\S*$/i, 'STM32f$1xx');
        var devmem = deviceName.replace(/^stm32f(\d\d\d)\S(\S)\S*$/i, 'STM32f$1x$2').toLowerCase();
        var devdef = devmem.replace(/^(stm32f\d\d\dx)8/i, "$1b");
        var m3 = /^stm32f[1]\d\d/i.test(deviceName);
        var archFlags = [];
        if(m3) archFlags = ["-mthumb", "-mcpu=cortex-m3"];
        var driver = "unknown";
        if(/^stm32f100/i.test(deviceName)) driver = "stm32f100";
        else if(/^stm32f10\d/i.test(deviceName)) driver = "stm32f10x";
        return {
            archFlags : archFlags,
            ldScript  : dn + "_FLASH.ld",
            family    : family,
            devmem    : devmem,
            startup   : devdef,
            defines   : [devdef.toUpperCase().replace(/X/, 'x')],
            driver    : driver,
            driverDir : path + "/drivers/" + driver + "/",
            includePaths : [
                path + "/cmsis/Include",
                path + "/cmsis/Device/ST/" + family + "/Include",
                path + "/drivers/" + driver
            ]

        }
    }

    //cpp.linkerFlags: device.archFlags
    //cpp.includePaths : device.includePaths
    //cpp.defines: ["STM32F103xB"]

    Group {
        name : "CMSIS"
        prefix: path + "/cmsis/Device/ST/" + device.family + "/Source/Templates/"
        files : [
            "gcc/startup_" + device.startup + ".s",
            "system_" + device.family.toLowerCase() + ".c"
        ]
    }

    Group {
        name : "Linker"
        fileTags: "linkerscript"
        prefix : path + "/linker/"
        files : device.ldScript
    }
}
