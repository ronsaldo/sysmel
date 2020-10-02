Class {
	#name : #GenericRay2d,
	#superclass : #GenericGeometricShape,
	#instVars : [
		'origin',
		'direction',
		'inverseDirection',
		'tmin',
		'tmax'
	],
	#category : #'SysmelPharoCompatLayer-Core-Math-Geometry'
}

{ #category : #constructing }
GenericRay2d class >> from: startPoint to: endPoint [
	| vector vectorLength direction |
	vector := endPoint - startPoint.
	vectorLength := vector length.
	direction := vector / (vectorLength ~= 0 ifTrue: [vectorLength] ifFalse: [1]).
	^ self origin: startPoint direction: direction tmin: 0 tmax: vectorLength.
]

{ #category : #constructing }
GenericRay2d class >> isAbstract [
	^ self == GenericRay2d
]

{ #category : #constructing }
GenericRay2d class >> origin: origin direction: direction [
	^ self origin: origin direction: direction tmin: 0 tmax: Float infinity
]

{ #category : #constructing }
GenericRay2d class >> origin: origin direction: direction tmax: tmax [
	^ self origin: origin direction: direction tmin: 0 tmax: tmax
]

{ #category : #constructing }
GenericRay2d class >> origin: origin direction: direction tmin: tmin tmax: tmax [
	^ self basicNew
		origin: origin;
		direction: direction;
		inverseDirection: 1.0 / direction;
		tmin: tmin;
		tmax: tmax;
		yourself
]

{ #category : #accessing }
GenericRay2d >> direction [
	^ direction
]

{ #category : #accessing }
GenericRay2d >> direction: anObject [
	direction := anObject implicitCastTo: self class vector2Type
]

{ #category : #initialization }
GenericRay2d >> initialize [
	origin := self class vector2Type zeros.
	direction := self class vector2Type zeros.
	inverseDirection := self class vector2Type zeros.
	self tmin: 0; tmax: 0
]

{ #category : #accessing }
GenericRay2d >> inverseDirection [
	^ inverseDirection
]

{ #category : #accessing }
GenericRay2d >> inverseDirection: anObject [
	inverseDirection := anObject implicitCastTo: self class vector2Type
]

{ #category : #accessing }
GenericRay2d >> origin [
	^ origin
]

{ #category : #accessing }
GenericRay2d >> origin: anObject [
	origin := anObject implicitCastTo: self class vector2Type
]

{ #category : #'as yet unclassified' }
GenericRay2d >> pointAtDistance: distance [
	^ origin + (direction * distance)
]

{ #category : #accessing }
GenericRay2d >> tmax [
	^ tmax
]

{ #category : #accessing }
GenericRay2d >> tmax: anObject [
	tmax := anObject implicitCastTo: self class scalarType
]

{ #category : #accessing }
GenericRay2d >> tmin [
	^ tmin
]

{ #category : #accessing }
GenericRay2d >> tmin: anObject [
	tmin := anObject implicitCastTo: self class scalarType
]