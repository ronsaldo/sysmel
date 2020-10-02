Class {
	#name : #GenericRectangle,
	#superclass : #GenericGeometricShape,
	#instVars : [
		'min',
		'max'
	],
	#category : #'SysmelPharoCompatLayer-Core-Math-Geometry'
}

{ #category : #constructing }
GenericRectangle class >> center: center extent: extent [
	^ self center: center halfExtent: extent / 2
]

{ #category : #constructing }
GenericRectangle class >> center: center halfExtent: halfExtent [
	^ self basicNew
		min: center - halfExtent;
		max: center + halfExtent;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> empty [
	^ self basicNew
		min: self vector3Type infinityOrMaxValue;
		max: self vector3Type negativeInfinityOrMinValue;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> extent: extent [
	^ self basicNew
		min: 0;
		max: extent;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> fullSpace [
	^ self basicNew
		min: self vector3Type negativeInfinityOrMinValue;
		max: self vector3Type infinityOrMaxValue;
		yourself
]

{ #category : #testing }
GenericRectangle class >> isAbstract [
	^ self == GenericBox
]

{ #category : #constructing }
GenericRectangle class >> min: min extent: extent [
	^ self basicNew
		min: min;
		max: min + extent;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> min: min max: max [
	^ self basicNew
		min: min;
		max: max;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> point: aPoint [
	^ self basicNew
		min: aPoint;
		max: aPoint;
		yourself
]

{ #category : #constructing }
GenericRectangle class >> unitBox [
	^ self min: 0.5 max: 0.5
]

{ #category : #converting }
GenericRectangle >> asBox [
	^ self class boxType min: min asVector3 max: max asVector3
]

{ #category : #accessing }
GenericRectangle >> center [
	^ min + ((max - min) / 2)
]

{ #category : #serialization }
GenericRectangle >> encodeForJSON [
	^ OrderedDictionary newFromPairs: {
		#min . min encodeForJSON.
		#max . max encodeForJSON.
	}
]

{ #category : #accessing }
GenericRectangle >> extent [
	^ max - min
]

{ #category : #accessing }
GenericRectangle >> halfExtent [
	^ (max - min) / 2
]

{ #category : #accessing }
GenericRectangle >> height [
	^ max y - min y
]

{ #category : #initialization }
GenericRectangle >> initialize [
	min := self class vector2Type zeros.
	max := self class vector2Type zeros.
]

{ #category : #testing }
GenericRectangle >> isEmpty [
	^ min x > max x or: [ min y > max y ]
]

{ #category : #accessing }
GenericRectangle >> max [
	^ max
]

{ #category : #accessing }
GenericRectangle >> max: anObject [
	max := anObject implicitCastTo: self class vector2Type
]

{ #category : #accessing }
GenericRectangle >> min [
	^ min
]

{ #category : #accessing }
GenericRectangle >> min: anObject [
	min := anObject implicitCastTo: self class vector2Type
]

{ #category : #printing }
GenericRectangle >> printOn: aStream [
	self isEmpty ifTrue: [ 
		^ aStream nextPutAll: self class name; nextPutAll: ' empty'
	].

	aStream nextPut: $(; nextPutAll: self class name;
		nextPutAll:  ' min: '; print: min;
		nextPutAll:  ' max: '; print: max;
		nextPut: $)
]

{ #category : #transforming }
GenericRectangle >> translatedBy: translation [
	^ self class min: min + translation max: max + translation
]

{ #category : #accessing }
GenericRectangle >> width [
	^ max x - min x
]