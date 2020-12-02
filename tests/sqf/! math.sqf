[   ["assertEqual",      { + 1 }, 1],
    ["assertEqual",      { - 1 }, -1],
    ["assertEqual",      { +1 + +1 }, 2],
    ["assertEqual",      { -1 - -1 }, 0],
    ["assertEqual",      { -1 * -1 }, 1],
    ["assertEqual",      { +1 * +1 }, 1],
    ["assertEqual",      { 2 ^ -1 }, 0.5],
    ["assertEqual",      { 1 / 0 }, 0],
    ["assertEqual",      { 1 / 1 }, 1],
    ["assertEqual",      { 0.001 / 0 }, 0],
    ["assertEqual",      { abs 1337.7 }, 1337.7],
    ["assertEqual",      { round deg 0.7 }, 40],
    ["assertEqual",      { log 1337 }, 3.1261312961578369],
    ["assertEqual",      { pi }, 3.1415927410125732],
    ["assertEqual",      { rad 180 }, pi],
    ["assertEqual",      { deg pi }, 180],
    ["assertEqual",      { sin 30 }, 0.5],
    ["assertEqual",      { asin 0.5 }, 30],
    ["assertEqual",      { str cos 60 }, str 0.5],
    ["assertEqual",      { acos 0.5 },  60],
    ["assertEqual",      { exp 2 }, 7.3890562057495117],
    ["assertEqual",      { sqrt 220 }, 14.8323965072631836],
    ["assertEqual",      { tan 45 }, 1],
    ["assertEqual",      { atan 1 }, 45],
    ["assertEqual",      { str (5 atan2 3) }, str 59.0362],
    ["assertEqual",      { 5 min 20 }, 5],
    ["assertEqual",      { 5 max 20 }, 20],
    ["assertEqual",      { floor 133.7 }, 133],
    ["assertEqual",      { ceil 133.7 }, 134],
    ["assertEqual",      { ln 220 }, 5.3936276435852051],
    ["assertEqual",      { 1337 mod 220 }, 17],
    ["assertEqual",      { round 133.7 }, 134],
    ["assertEqual",      { [1,2,3] vectorDotProduct [3,2,1] }, 10],
    ["assertEqual",      { vectorMagnitude [1,2,3] }, 3.7416574954986572],
    ["assertEqual",      { vectorMagnitudeSqr [1,2,3] }, 14],
    ["assertEqual",      { [1,2,3] vectorDistanceSqr [3,2,1] }, 8],
    ["assertEqual",      { [1,2,3] vectorDistance [3,2,1] }, 2.82842707633972170000],
    ["assertEqual",      { [1,2,3] vectorAdd [3,2,1] }, [4,4,4]],
    ["assertEqual",      { ([1,2,3] vectorCos [3,2,1]) toFixed 5 }, "0.71429"],
    ["assertEqual",      { [1,2,3] vectorDiff [3,2,1] },[-2,0,2]],
    ["assertEqual",      { [1,2,3] vectorCrossProduct [3,2,1] }, [-4,8,-4]],
    ["assertEqual",      { [1,2,3] vectorMultiply 3 }, [3,6,9]],
    ["assertEqual",      { private _r = vectorNormalized [3,2,1]; [_r # 0 toFixed 5, _r # 1 toFixed 5, _r # 2 toFixed 5] }, ["0.80178","0.53452","0.26726"]],
    ["assertEqual",      { vectorNormalized [0,0,0] }, [0,0,0]],
    ["assertEqual",      { 5 toFixed 2 }, "5.00"],
    ["assertEqual",      { 5 toFixed 25 }, "5.00000000000000000000"],
    ["assertEqual",      { 5 toFixed -2 }, "5"],
    ["assertNil",        { toFixed 2; toFixed -1 }, "5.00"],
    ["assertNil",        { random 5; nil }],
    ["assertException",  { [1,2,3] vectorAdd [true,2,1] }],
    ["assertException",  { [1,2,3] vectorCos [true,2,1] }],
    ["assertException",  { [1,2,3] vectorDotProduct [true,2,1] }],
    ["assertException",  { [1,2,3] vectorCrossProduct [true,2,1] }],
    ["assertException",  { [1,2,3] vectorDistance [true,2,1] }],
    ["assertException",  { [1,2,3] vectorDistanceSqr [true,2,1] }],
    ["assertException",  { [true,2,3] vectorMultiply 0 }],
    ["assertException",  { [1,2,3] vectorDiff [true,2,1] }],
    ["assertException",  { vectorMagnitude [true,2,1] }],
    ["assertException",  { vectorMagnitudeSqr [true,2,1] }],
    ["assertException",  { vectorNormalized [true,2,1] }],
    ["assertFalse",      { private _arr = [1,2,3]; private _nested = [_arr]; private _copy = +_nested; (_copy select 0) set [2, 1]; _nested isEqualTo _copy }, "5.00"]
]