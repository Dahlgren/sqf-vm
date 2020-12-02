[   ["setup",           { private _obj1 = "CAManBase" createVehicle [0,0,0]; private _obj2 = "CAManBase" createVehicle [0,0,0]; [] call _this; deleteVehicle _obj1; deleteVehicle _obj2; }, 1],
    ["assertEqual",     { [0,0,4] distance2d [0,1,1] }, 1],
    ["assertEqual",     { [0,0,3] distance2d [2,0,2] }, 2],
    ["assertEqual",     { [0,3,2] distance2d [0,0,3] }, 3],
    ["assertEqual",     { [4,0,1] distance2d [0,0,4] }, 4],
    ["assertEqual",     { _obj1 setPos [0,0,4]; private _res = _obj1 distance2d [0,1,1]; _res }, 1],
    ["assertEqual",     { _obj1 setPos [0,0,3]; private _res = _obj1 distance2d [2,0,2]; _res }, 2],
    ["assertEqual",     { _obj1 setPos [0,3,2]; private _res = _obj1 distance2d [0,0,3]; _res }, 3],
    ["assertEqual",     { _obj1 setPos [4,0,1]; private _res = _obj1 distance2d [0,0,4]; _res }, 4],
    ["assertEqual",     { _obj1 setPos [0,0,4]; private _res = [0,1,1] distance2d _obj1; _res }, 1],
    ["assertEqual",     { _obj1 setPos [0,0,3]; private _res = [2,0,2] distance2d _obj1; _res }, 2],
    ["assertEqual",     { _obj1 setPos [0,3,2]; private _res = [0,0,3] distance2d _obj1; _res }, 3],
    ["assertEqual",     { _obj1 setPos [4,0,1]; private _res = [0,0,4] distance2d _obj1; _res }, 4],
    ["assertEqual",     { _obj1 setPos [0,0,4]; _obj2 setPos [0,1,1]; private _res = _obj1 distance2d _obj2; _res }, 1],
    ["assertEqual",     { _obj1 setPos [0,0,3]; _obj2 setPos [2,0,2]; private _res = _obj1 distance2d _obj2; _res }, 2],
    ["assertEqual",     { _obj1 setPos [0,3,2]; _obj2 setPos [0,0,3]; private _res = _obj1 distance2d _obj2; _res }, 3],
    ["assertEqual",     { _obj1 setPos [4,0,1]; _obj2 setPos [0,0,4]; private _res = _obj1 distance2d _obj2; _res }, 4]
]