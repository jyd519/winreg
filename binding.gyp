{
    "targets": [
        {
            "target_name": "winreg",
            "sources": ["winreg.cc"],
            "defines": ["UNICODE", "_UNICODE"],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ]
        }
    ]
}
