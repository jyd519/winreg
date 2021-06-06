{
  "targets": [
  {
    "target_name": "winreg",
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    "msvs_settings": {
      "VCCLCompilerTool": { "ExceptionHandling": 1 },
    },
    "sources": ["winreg.cc"],
    "defines": ["UNICODE", "_UNICODE"],
    'include_dirs': ['<!@(node -p "require(\'node-addon-api\').include")'],
    'dependencies': ['<!(node -p "require(\'node-addon-api\').gyp")'],
  },
  {
    'target_name': 'action_after_build',
    'type': 'none',
    'dependencies': [ 'winreg' ],
    'copies': [
      {
        'files': [ '<(PRODUCT_DIR)/winreg.node' ],
        'destination': '<(module_root_dir)/<(target_arch)'
      }
    ]
  }
  ]
}
