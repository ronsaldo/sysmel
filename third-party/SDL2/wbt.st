self platform name selectCase: {
    #linux -> [
        self importLibrary: 'SDL2' fromDebianPackage: 'libsdl2-dev'.
    ].
    #osx -> [

    ].
    #windows -> [
        | folder |
        folder := 'windows/' , self architecture name.
        self importLibrary: 'SDL2' fromLibFolder: folder , '/lib' binFolder: folder , '/bin'
    ].
    [true] -> [].
}.