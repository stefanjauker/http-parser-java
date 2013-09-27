{
    'includes': {
        'config.gypi',
    },
    'variables': {
        'HTTPPARSER_HOME%': '<(SOURCE_HOME%)/deps/http_parser',
        'HTTPPARSER_PATH%': 'out/<(target)/lib.target',
    },
    'target_defaults': {
        'default_configuration': '<(target)',
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG', '_DEBUG' ],
                'cflags': [ '-g', '-O0' ],
                'conditions': [
                    ['OS == "win"', {
                        'msvs_settings': {
                            'VCLinkerTool': {
                                'GenerateDebugInformation': 'true',
                            },
                        },
                    }],
                    [ 'OS=="linux"', {
                    }],
                    [ 'OS=="mac"', {
                    }],
                ],
            },
            'Release': {
                'defines': [ 'NDEBUG' ],
                'conditions': [
                    ['OS == "win"', {
                    }],
                    [ 'OS=="linux"', {
                    }],
                    [ 'OS=="mac"', {
                    }],
                ],
            },
        }
    },
    'targets': [
        {
            'target_name': 'http-parser-java',
            'type': 'shared_library',
            'defines': [
            ],
            'dependencies': [
            ],
            'include_dirs': [
                '<(JAVA_HOME)/include',
                '<(HTTPPARSER_HOME)',
                '<(HTTPPARSER_PATH)/../obj.target/http-parser-java',
            ],
            'conditions': [
                ['OS == "linux"', {
                    'sources': [
                        'http_parser.c',
                        'parser.cpp',
                    ],
                    'libraries': [
                    ],
                    'defines': [
                        '__POSIX__',
                    ],
                    'cflags': [
                        '-fPIC',
                    ],
                    'ldflags': [
                    ],
                    'include_dirs': [
                        '<(JAVA_HOME)/include/linux',
                    ],
                }],
                ['OS == "mac"', {
                    'target_conditions': [
                        ['target_arch=="x64"', {
                            'xcode_settings': {'ARCHS': ['x86_64']},
                        }]
                    ],
                    'sources': [
                        '<(HTTPPARSER_PATH)/../obj.target/http-parser-java/http_parser.c',
                        '<(HTTPPARSER_PATH)/../obj.target/http-parser-java/parser.cpp',
                    ],
                    'libraries': [
                    ],
                    'defines': [
                        '__POSIX__',
                    ],
                    'include_dirs': [
                        '<(JAVA_HOME)/include/darwin',
                    ],
                }],
                ['OS == "win"', {
                    'target_conditions': [
                        ['target_arch=="x64"', {
                            'msvs_configuration_platform': 'x64'
                        }]
                    ],
                    'sources': [
                        '<(HTTPPARSER_PATH)/../obj.target/http-parser-java/http_parser.c',
                        '<(HTTPPARSER_PATH)/../obj.target/http-parser-java/parser.cpp',
                    ],
                    'defines': [
                        '_WIN32',
                    ],
                    'cflags': [
                    ],
                    'ldflags': [
                    ],
                    'include_dirs': [
                        '<(JAVA_HOME)/include/win32',
                    ],
                    'libraries': [
                    ],
                }]
            ],
            'actions': [
            ],
        },

    ],

}
