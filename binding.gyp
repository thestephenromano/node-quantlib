{
    "targets": [{
        "target_name": "node-quantlib",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "sources": [ "addon.cc" ],
        "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
    ],
    "conditions": [
        ["OS=='mac'", {
            "xcode_settings": {
                "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                "GCC_ENABLE_CPP_RTTI": "YES",
            }
        }],
        ["OS=='linux'", {
            'cflags_cc!': ['-fno-rtti'],
            'cflags_cc+': ['-frtti'],
        }]
    ],
        "libraries": ["/usr/local/lib/libQuantLib.so.0.0.0"],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }]
}