{
    "targets": [
        {
            "target_name": "electron_application_desktop_toolbar",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["src/electron_application_desktop_toolbar.cpp"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
            'xcode_settings': {
                'OTHER_CPLUSPLUSFLAGS': ['-std=c++14', '-stdlib=libc++'],
            },
        }
    ]
}
