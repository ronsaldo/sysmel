Class {
	#name : #GenericRay3d,
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

{ #category : #'as yet unclassified' }
GenericRay3d class >> from: startPoint to: endPoint [
	| vector vectorLength direction |
	vector := endPoint - startPoint.
	vectorLength := vector length.
	direction := vector / (vectorLength ~= 0 ifTrue: [vectorLength] ifFalse: [1]).
	^ self origin: startPoint direction: direction tmin: 0 tmax: vectorLength.
]

{ #category : #testing }
GenericRay3d class >> isAbstract [
	^ self == GenericRay3d
]

{ #category : #'as yet unclassified' }
GenericRay3d class >> origin: origin direction: direction [
	^ self origin: origin direction: direction tmin: 0 tmax: Float infinity
]

{ #category : #'as yet unclassified' }
GenericRay3d class >> origin: origin direction: direction tmax: tmax [
	^ self origin: origin direction: direction tmin: 0 tmax: tmax
]

{ #category : #'as yet unclassified' }
GenericRay3d class >> origin: origin direction: direction tmin: tmin tmax: tmax [
	^ self basicNew
		origin: origin;
		direction: direction;
		inverseDirection: 1.0 / direction;
		tmin: tmin;
		tmax: tmax;
		yourself
]

{ #category : #accessing }
GenericRay3d >> direction [
	^ direction
]

{ #category : #accessing }
GenericRay3d >> direction: anObject [
	direction := anObject implicitCastTo: self class vector3Type
]

{ #category : #initialization }
GenericRay3d >> initialize [
	origin := self class vector3Type zeros.
	direction := self class vector3Type zeros.
	inverseDirection := self class vector3Type zeros.
	self tmin: 0; tmax: 0
]

{ #category : #accessing }
GenericRay3d >> inverseDirection [
	^ inverseDirection
]

{ #category : #accessing }
GenericRay3d >> inverseDirection: anObject [
	inverseDirection := anObject implicitCastTo: self class vector3Type
]

{ #category : #accessing }
GenericRay3d >> origin [
	^ origin
]

{ #category : #accessing }
GenericRay3d >> origin: anObject [
	origin := anObject implicitCastTo: self class vector3Type
]

{ #category : #'as yet unclassified' }
GenericRay3d >> pointAtDistance: distance [
	^ origin + (direction * distance)
]

{ #category : #accessing }
GenericRay3d >> tmax [
	^ tmax
]

{ #category : #accessing }
GenericRay3d >> tmax: anObject [
	tmax := anObject implicitCastTo: self class scalarType
]

{ #category : #accessing }
GenericRay3d >> tmin [
	^ tmin
]

{ #category : #accessing }
GenericRay3d >> tmin: anObject [
	tmin := anObject implicitCastTo: self class scalarType
]