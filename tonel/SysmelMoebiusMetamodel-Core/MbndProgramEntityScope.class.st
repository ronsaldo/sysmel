Class {
	#name : #MbndProgramEntityScope,
	#superclass : #MbndAbstractScope,
	#instVars : [
		'programEntity'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Scope'
}

{ #category : #accessing }
MbndProgramEntityScope >> compilationEnvironment [
	^ programEntity compilationEnvironment
]

{ #category : #accessing }
MbndProgramEntityScope >> compilationTarget [
	^ programEntity compilationTarget
]

{ #category : #accessing }
MbndProgramEntityScope >> defaultArgumentType [
	programEntity isMbndType ifTrue: [ ^ programEntity defaultArgumentType ].
	^ super defaultArgumentType
]

{ #category : #accessing }
MbndProgramEntityScope >> defaultReturnType [
	programEntity isMbndType ifTrue: [ ^ programEntity defaultReturnType ].
	^ super defaultReturnType
]

{ #category : #accessing }
MbndProgramEntityScope >> defaultVariableType [
	programEntity isMbndType ifTrue: [ ^ programEntity defaultVariableType ].
	^ super defaultVariableType
]

{ #category : #serialization }
MbndProgramEntityScope >> encodeFieldsForMessagePackWith: graphEncoder [
	^ (super encodeFieldsForMessagePackWith: graphEncoder) , {
		#programEntity . programEntity encodeGraphForMessagePackWith: graphEncoder.
	}
]

{ #category : #formatting }
MbndProgramEntityScope >> formatPositionForErrorMessageInMacroExpansion: sourcePosition [
	programEntity definitionPosition asActualSourcePosition isEmptySourcePosition ifFalse: [ 
		programEntity definitionPosition sourceTextPosition ifNotNil: [
			^ programEntity definitionPosition sourceTextPosition printString , ': '
		]
	].
	^ programEntity qualifiedName , ': '
]

{ #category : #testing }
MbndProgramEntityScope >> isProgramEntityScope [
	^ true
]

{ #category : #'macro expansion' }
MbndProgramEntityScope >> lookupMacroSelector: selector [
	(programEntity lookupMacroSelector: selector) ifNotNil: [ :macroMethod |
		^ macroMethod
	].

	^ super lookupMacroSelector: selector
]

{ #category : #'macro expansion' }
MbndProgramEntityScope >> lookupOverloadedSelector: selector [
	(programEntity lookupOverloadedSelector: selector) ifNotNil: [ :method |
		^ method
	].

	^ super lookupOverloadedSelector: selector
]

{ #category : #'as yet unclassified' }
MbndProgramEntityScope >> lookupProgramEntityForNewDefinitions [
	^ programEntity
]

{ #category : #'macro expansion' }
MbndProgramEntityScope >> lookupSymbol: aSymbol [
	^ programEntity lookupSymbol: aSymbol
]

{ #category : #serialization }
MbndProgramEntityScope >> materializeWithGraphDecoder: graphDecoder withFields: encodedFields [
	super materializeWithGraphDecoder: graphDecoder withFields: encodedFields.
	programEntity := encodedFields at: #programEntity.

]

{ #category : #accessing }
MbndProgramEntityScope >> programEntity [
	^ programEntity
]

{ #category : #accessing }
MbndProgramEntityScope >> programEntity: anObject [
	programEntity := anObject
]
