#ifndef PLC_ADDRESSES_H
#define PLC_ADDRESSES_H


namespace PLCDB {
    constexpr int CONTROL = 1;   // 控制/状态信号所在DB
    constexpr int COARSE  = 2;   // 粗定位数据所在DB
    constexpr int FINE    = 3;   // 精定位数据所在DB
}

struct PlcBitAddr {
    int db;
    int byteOffset;
    int bitOffset;
};


struct PlcFloatAddr {
    int db;
    int byteOffset;
};

// 控制信号
namespace PLCBits {
    constexpr PlcBitAddr StartScan      { PLCDB::CONTROL, 0, 0 }; // DB1.DBX0.0
    constexpr PlcBitAddr CoarseComplete { PLCDB::CONTROL, 0, 1 }; // DB1.DBX0.1
    constexpr PlcBitAddr FineComplete   { PLCDB::CONTROL, 0, 2 }; // DB1.DBX0.2
    constexpr PlcBitAddr EmergencyStop  { PLCDB::CONTROL, 0, 3 }; // DB1.DBX0.3
    constexpr PlcBitAddr SystemReady    { PLCDB::CONTROL, 0, 4 }; // DB1.DBX0.4
}

namespace PLCCoarse {
    constexpr PlcFloatAddr X  { PLCDB::COARSE, 0  };  // DB2.DBD0
    constexpr PlcFloatAddr Y  { PLCDB::COARSE, 4  };  // DB2.DBD4
    constexpr PlcFloatAddr Z  { PLCDB::COARSE, 8  };  // DB2.DBD8
    constexpr PlcFloatAddr Rx { PLCDB::COARSE, 12 };  // DB2.DBD12
    constexpr PlcFloatAddr Ry { PLCDB::COARSE, 16 };  // DB2.DBD16
    constexpr PlcFloatAddr Rz { PLCDB::COARSE, 20 };  // DB2.DBD20
}


namespace PLCFine {
    constexpr PlcFloatAddr dX { PLCDB::FINE, 0 };   // DB3.DBD0
    constexpr PlcFloatAddr dY { PLCDB::FINE, 4 };   // DB3.DBD4
    constexpr PlcFloatAddr dRz{ PLCDB::FINE, 8 };   // DB3.DBD8
}


#endif // PLC_ADDRESSES_H


