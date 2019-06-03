Class {
	#name : #MbndAggregateTypeWithFields,
	#superclass : #MbndAggregateType,
	#instVars : [
		'fields',
		'slotLayout'
	],
	#category : #'SysmelMoebiusMetamodel-Core-Type'
}

{ #category : #accessing }
MbndAggregateTypeWithFields class >> typeName [
	^ #_AggregateTypeWithFields
]

{ #category : #adding }
MbndAggregateTypeWithFields >> addField: aField [
	self addChild: aField.

	aField name isMbndAnonymousSymbol ifFalse: [ 
		self setLocalSymbol: aField name value: aField.
		aField isPublic ifTrue: [
			self createPublicFieldAccessors: aField
		].
	].

	self fields add: aField.

]

{ #category : #layout }
MbndAggregateTypeWithFields >> buildSlotLayout [
	| builder |
	builder := MbndAggregateSlotLayoutBuilder new.
	self buildSlotLayoutWith: builder.
	slotLayout := builder finish.
]

{ #category : #layout }
MbndAggregateTypeWithFields >> buildSlotLayoutWith: builder [
	self buildSuperTypesSlotLayoutWith: builder.
	self fields do: [ :field |
		builder addField: field
	].
]

{ #category : #layout }
MbndAggregateTypeWithFields >> buildSuperTypesSlotLayoutWith: builder [
]

{ #category : #adding }
MbndAggregateTypeWithFields >> createPublicFieldAccessors: aField [
	self macroMethodDict
		at: aField name asSymbol ifAbsentPut: [
			MbndFieldGetterAccessorMacro new field: aField
		];
		at: (aField name , ':') asSymbol ifAbsentPut: [
			MbndFieldSetterAccessorMacro new field: aField
		].
]

{ #category : #'default values' }
MbndAggregateTypeWithFields >> defaultValueAt: aPosition [
	| valueSlots |
	self isDefined ifFalse: [ 
		self error: 'Cannot produce a value for incomplete type {1}.' format: { self asString } at: aPosition
	].

	valueSlots := self slotLayout slots collect: [ :slot | (slot defaultValueAt: aPosition) unwrapMbndValue].
	^ MbndAggregateTypeInstanceValue new
		type: self;
		slots: valueSlots;
		yourself
]

{ #category : #'semantic analysis' }
MbndAggregateTypeWithFields >> ensureSemanticAnalysis [
	super ensureSemanticAnalysis.
	self isDefined ifTrue: [ 
		self ensureSlotLayoutIsComputed.
	] ifFalse: [ 
	]

]

{ #category : #layout }
MbndAggregateTypeWithFields >> ensureSlotLayoutIsComputed [
	(self hasFlag: #computingSlotLayout) ifTrue: [ 
		self error: 'Type with circular slot layout.'
	].
	self
		addFlag: #computingSlotLayout;
		buildSlotLayout;
		removeFlag: #computingSlotLayout.

]

{ #category : #accessing }
MbndAggregateTypeWithFields >> fields [
	^ fields ifNil: [ fields := OrderedCollection new ]
]

{ #category : #'meta builder factories' }
MbndAggregateTypeWithFields >> getExtraMetaBuilderFactoryList [
	^ super getExtraMetaBuilderFactoryList ,  { 
		#field . MbndFieldMetaBuilder metaBuilderFactory.
	}
]

{ #category : #accessing }
MbndAggregateTypeWithFields >> instanceAlignment [
	<compileTime>
	slotLayout ifNil: [self ensureSlotLayoutIsComputed].
	^ slotLayout instanceAlignment
]

{ #category : #accessing }
MbndAggregateTypeWithFields >> instanceSize [
	<compileTime>
	slotLayout ifNil: [self ensureSlotLayoutIsComputed].
	^ slotLayout instanceSize
]

{ #category : #'semantic analysis' }
MbndAggregateTypeWithFields >> isDefined [
	^ self hasFlag: #defined
]

{ #category : #accessing }
MbndAggregateTypeWithFields >> slotLayout [
	slotLayout ifNil: [
		self isDefined ifFalse: [ 
			self error: 'Cannot instantiate or inherit from opaque type.'
		].
		self ensureSlotLayoutIsComputed
	].
	^ slotLayout
]