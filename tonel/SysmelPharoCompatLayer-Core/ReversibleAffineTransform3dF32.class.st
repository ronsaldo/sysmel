Class {
	#name : #ReversibleAffineTransform3dF32,
	#superclass : #GenericReversibleAffineTransform3d,
	#category : #'SysmelPharoCompatLayer-Core-Math'
}

{ #category : #accessing }
ReversibleAffineTransform3dF32 class >> scalarType [
	^ Float32
]

{ #category : #converting }
ReversibleAffineTransform3dF32 >> asReversibleAffineTransform3dF32 [
	^ self
]
