Class {
	#name : #Float64x2,
	#superclass : #SysmelPrimitiveVector2Type,
	#category : #'SysmelPharoCompatLayer-Core-PrimitiveTypes'
}

{ #category : #accessing }
Float64x2 class >> elementType [
	^ Float64
]

{ #category : #converting }
Float64x2 >> asFloat64x2 [
	^ self
]
