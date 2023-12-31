#include "pci.hpp"

#include "asmfunc.h"

namespace { //internal linkage(= can't be accessed by other file)
    using namespace pci;

    uint32_t MakeAddress(uint8_t bus, uint8_t device, //make config address
                        uint8_t function, uint8_t reg_addr) {
        //lambda [capture](argument){body} lambda function(ref p144) bcz it can be defined in other function!
        //set this lambda function to variable named shl
        auto shl = [](uint32_t x, unsigned int bits) { 
            return x << bits;
        };

        return shl(1,31) //Enable
            | shl(bus, 16) //bus for example, 128(=1000000) is shifted right
            | shl(device, 11) //device
            | shl(function, 8) //function
            | (reg_addr & 0xfcu); //offset(2bit is 0)
    }

    Error AddDevice(const Device& device) {
        if (num_device == devices.size()){
            return MAKE_ERROR(Error::kFull);
        }

        devices[num_device] = device;
        ++num_device;
        return MAKE_ERROR(Error::kSuccess);
    }

    Error ScanBus(uint8_t bus);

    Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function) { //register (bus,device,function) to devices array.
        auto class_code = ReadClassCode(bus,device,function);
        auto header_type = ReadHeaderType(bus,device,function);
        Device dev{bus, device, function, header_type, class_code};
        if (auto err = AddDevice(dev)) {
            return err;
        }

        if(class_code.Match(0x06u, 0x04u)) {
            auto bus_numbers = ReadBusNumbers(bus, device, function);
            uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
            return ScanBus(secondary_bus);
        }

        return MAKE_ERROR(Error::kSuccess);
    }

    Error ScanDevice(uint8_t bus, uint8_t device) {
        if (auto err = ScanFunction(bus, device, 0)) { //the result of expression is err!
            return err;
        }
        if ( IsSingleFunctionDevice(ReadHeaderType(bus, device, 0))) {
            return MAKE_ERROR(Error::kSuccess);
        }

        for (uint8_t function = 1; function < 8; ++ function) {
            if ( ReadVendorId(bus, device, function) == 0xffffu) {
                continue;
            }
            if(auto err = ScanFunction(bus,device,function)) {
                return err;
            }
        }
        return MAKE_ERROR(Error::kSuccess);
    }

    Error ScanBus(uint8_t bus) {
        for (uint8_t device = 0; device < 32; ++ device){ // there are at most 32 devices connected to PCI device.
            if(ReadVendorId(bus,device,0) == 0xffffu) { //to check whether there is a device.
                continue;
            }
            if(auto err = ScanDevice(bus, device)) {
                return err;
            }
        }
        return MAKE_ERROR(Error::kSuccess);
    }

}

namespace pci {
    void WriteAddress(uint32_t address) { //output the data in eax to the area
        IoOut32(kConfigAddress, address); //address is set to the memory in kConfigAddress.
    }

    void WriteData(uint32_t value) { //input the value 
        IoOut32(kConfigData, value); //value is set to the memory in kConfigData.
    }

    uint32_t ReadData() {
        return IoIn32(kConfigData); //import the data in kConfigData address.
    }

    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function) {
        //if I set the kConfigAddress , kConfigData which corrspond with kConfigAddress is imported automatically??
        WriteAddress(MakeAddress(bus, device, function, 0x00));
        return ReadData() & 0xffffu;
    }

    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function) {
        WriteAddress(MakeAddress(bus, device, function, 0x00));
        return ReadData() >> 16;
  }

    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
        WriteAddress(MakeAddress(bus, device, function, 0x0c));
        return (ReadData() >> 16) & 0xffu;
  }

    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function) {
        WriteAddress(MakeAddress(bus, device, function, 0x08));
        auto reg = ReadData();
        ClassCode cc;
        cc.base = (reg >> 24) & 0xffu;
        cc.sub = (reg >> 16) & 0xffu;
        cc.interface = (reg >> 8) & 0xffu;
        return cc;
  }

    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function) {
        WriteAddress(MakeAddress(bus, device, function, 0x18));
        return ReadData();
  }

    bool IsSingleFunctionDevice(uint8_t header_type) {
        return (header_type & 0x80u) == 0; //bit 7 of header type represents this device is multi function device
}

    Error ScanAllBus() { //search for device connented to PCI bus recursively.
        num_device = 0;

        auto header_type = ReadHeaderType(0,0,0); //necessairily there is 0 number funciton.
        if (IsSingleFunctionDevice(header_type)) { //it's in charge of 0 bus,
            return ScanBus(0);
        }

        for(uint8_t function = 0;function < 8; ++function){
            if(ReadVendorId(0,0,function) == 0xffffu) { //ref p143( to check if there is this "funciton".)
                continue;
            }

            if (auto err = ScanBus(function)) {
                return err;
            }
        }
        return MAKE_ERROR(Error::kSuccess);
    }

    uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr) {
        WriteAddress(MakeAddress(dev.bus, dev.device, dev.function, reg_addr));
        return ReadData();
    }

    void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value) {
        WriteAddress(MakeAddress(dev.bus, dev.device, dev.function, reg_addr));
        WriteData(value);
    }

    WithError<uint64_t> ReadBar(Device & device, unsigned int bar_index) {
        if (bar_index >= 6) {
            return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
        }

        const auto addr = CalcBarAddress(bar_index);
        const auto bar = ReadConfReg(device, addr);

        //32 bit address
        if ((bar & 4u) == 0) { 
            return {bar, MAKE_ERROR(Error::kSuccess)};
        }

        //64 bit address //we are supposed to use successive bar to express 64 bit address.
        if (bar_index >= 5) {
            return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
        }

        const auto bar_upper = ReadConfReg(device, addr + 4); //the unit of addr is 1 byte.
        return {
            bar | (static_cast<uint64_t>(bar_upper) << 32),
            MAKE_ERROR(Error::kSuccess)
        };

    }
}