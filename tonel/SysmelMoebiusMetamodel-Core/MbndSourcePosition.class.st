Class {
	#name : #MbndSourcePosition,
	#superclass : #MbndAbstractSourcePosition,
	#instVars : [
		'collection',
		'start',
		'stop',
		'line',
		'column',
		'endLine',
		'endColumn'
	],
	#classVars : [
		'EmptySourcePosition'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Parsing'
}

{ #category : #requirements }
MbndSourcePosition class >> empty [
	"
	EmptySourcePosition := nil
	"
	^ EmptySourcePosition ifNil: [ EmptySourcePosition := self new beReadOnlyObject; yourself ]
]

{ #category : #comparing }
MbndSourcePosition >> = other [
	^ self class == other class and:
		[ collection  == other collection and:
		[ start = other start and:
		[ stop = other stop ] ] ]
]

{ #category : #accessing }
MbndSourcePosition >> collection [
	^ collection
]

{ #category : #accessing }
MbndSourcePosition >> collection: anObject [
	collection := anObject.
	line := nil.
	column := nil.
	endLine := nil.
	endColumn := nil.
]

{ #category : #accessing }
MbndSourcePosition >> column [
	column ifNil: [ self fetchLineAndColumn ].
	^ column
]

{ #category : #accessing }
MbndSourcePosition >> endColumn [
	endColumn ifNil: [ self fetchEndLineAndColumn ].
	^ endColumn
]

{ #category : #accessing }
MbndSourcePosition >> endLine [
	endLine ifNil: [ self fetchEndLineAndColumn ].
	^ endLine
]

{ #category : #accessing }
MbndSourcePosition >> fetchEndLineAndColumn [
	| lineColumn |
	(collection isNil or: [ collection isString ]) ifTrue: [
		endLine := 0.
		endColumn := 0.
		^ self
	].

	lineColumn := collection lineColumnAt: stop.
	endLine := lineColumn first.
	endColumn := lineColumn second.
]

{ #category : #accessing }
MbndSourcePosition >> fetchLineAndColumn [
	| lineColumn |
	(collection isNil or: [collection isString]) ifTrue: [
		line := 0.
		column := 0.
		^ self
	].

	lineColumn := collection lineColumnAt: start.
	line := lineColumn first.
	column := lineColumn second.
]

{ #category : #accessing }
MbndSourcePosition >> fileName [
	^ collection ifNil: ['generated'] ifNotNil: [collection isString ifTrue: [collection] ifFalse: [ collection name]]
]

{ #category : #accessing }
MbndSourcePosition >> fullPrintOn: aStream [
	
]

{ #category : #comparing }
MbndSourcePosition >> hash [
	^ self class identityHash bitXor:
		( collection identityHash bitXor: 
		( start hash bitXor: stop hash))
]

{ #category : #initialization }
MbndSourcePosition >> initialize [
	super initialize.
	line := 0.
	column := 0.
	endLine := 0.
	endColumn := 0.
]

{ #category : #testing }
MbndSourcePosition >> isEmptySourcePosition [
	^ self = EmptySourcePosition
]

{ #category : #accessing }
MbndSourcePosition >> line [
	line ifNil: [ self fetchLineAndColumn ].
	^ line
]

{ #category : #accessing }
MbndSourcePosition >> originalSourceCode [
	^ self sourceCode
]

{ #category : #accessing }
MbndSourcePosition >> position [
	^ self
]

{ #category : #accessing }
MbndSourcePosition >> printOn: aStream [
	aStream nextPutAll: self fileName; nextPut: $:; print: self line; nextPut: $:; print: self column 
]

{ #category : #accessing }
MbndSourcePosition >> sourceCode [
	(collection isNil or: [collection isString]) ifTrue: [ ^ '' ].
	^ collection copyFrom: start to: stop
]

{ #category : #converting }
MbndSourcePosition >> sourceTextPosition [
	^ self
]

{ #category : #accessing }
MbndSourcePosition >> start [
	^ start
]

{ #category : #accessing }
MbndSourcePosition >> start: anObject [
	start := anObject
]

{ #category : #accessing }
MbndSourcePosition >> stop [
	^ stop
]

{ #category : #accessing }
MbndSourcePosition >> stop: anObject [
	stop := anObject
]

{ #category : #'as yet unclassified' }
MbndSourcePosition >> until: endPosition [
	^ self class new collection: collection; start: start; stop: endPosition stop; yourself
]
