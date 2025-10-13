{
    "targets": [
        {
            "target_name": "data_manager_addon",
            "cflags": ["-fPIC"],
            "cxxflags": ["-fexceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "<!@(echo `pkg-config --cflags sqlite3` | sed s/-I//g)",
            ],
            "libraries": ["-lsqlite3"],
            "sources": ["wrapper.cc", "data_manager.c"],
            "defines": ["NODE_ADDON_API_CPP_EXCEPTIONS"],
        }
    ]
}
