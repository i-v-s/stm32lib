import qbs
import "../Stm32Application.qbs" as Stm32Application

Project {
    name: "Template"
    minimumQbsVersion: "1.5.0"

    Stm32Application {
        deviceName: "STM32F100RBTx"
        consoleApplication: true
        type: ["application", "hex", "bin", "size", "elf", "disassembly"]

        Group {     // Properties for the produced executable
            name : "Sources"
            files : [
                "main.cpp"
            ]
        }
        cpp.defines: device.defines
        cpp.includePaths : device.includePaths

        cpp.linkerFlags: [
            "-lc",
            "-lnosys",
            "-specs=nosys.specs"
        ].concat(device.archFlags)

        cpp.commonCompilerFlags: [

        ].concat(device.archFlags)
    }
}
