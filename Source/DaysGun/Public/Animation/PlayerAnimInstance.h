// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"


UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	ELS_Idle UMETA(DisplayName = "Idle"),
	ELS_Walk UMETA(DisplayName = "Walk"),
	ELS_Run UMETA(DisplayName = "Run"),
	ELS_Crouch UMETA(DisplayName = "Crouch"),
	ELS_Jump UMETA(DisplayName = "Jump"),
};

struct OnEntryFlags
{
	bool IdleFlag = true;
	bool WalkFlag = false;
	bool RunFlag = false;
	bool CrouchFlag = false;
	bool JumpFlag = false;
};

UCLASS()
class DAYSGUN_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativePostEvaluateAnimation() override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	bool InCycleState();

	UFUNCTION(BlueprintImplementableEvent)
	bool InStartState();

	UFUNCTION(BlueprintImplementableEvent)
	bool StateMachineIsWalkStartState();

private:
	void SetReferences();
	void SetEssentialMovementData();
	void DetermineLocomotionState();
	void TrackLocomotionStates();

	void UpdateCharacterRotation();
	void ResetTransition();

	void UpdateVelocity();
	void UpdatePlayerInput();
	void UpdateInputVectorRotationRate();
	void UpdateLean();
	void UpdateAimOffset();

	void DetermineGroundLocomotionState();

	void UpdateStop();

	void UpdateOnWalkEntry();
	void UpdateOnRunToWalk();

	void UpdateOnRunEntry();
	void UpdateOnWalkToRun();

	void UpdateLocomotionValues();

	void CycleRotationBehavior();
	void StartRotationBehavior();

	void UpdateEntryVariables();

	void CalculateTargetRotationSmoothed();

protected:
#pragma region References
	UPROPERTY(BlueprintReadOnly, Category="References")
	class ABaseCharacter* PlayerRef;

	UPROPERTY(BlueprintReadOnly, Category="References")
	class UCharacterMovementComponent* CharacterMovementRef;
#pragma endregion

#pragma region EssentialData
	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector PrevVelocity;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	float MaxSpeed;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector InputVector;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector InputVectorLastFrame;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	bool StartedJumping;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FRotator ActorRotation;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	float InputVectorRotationRate;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector Acceleration;
	
	UPROPERTY(BlueprintReadOnly, Category="EssentialData|Lean")
	float LeanX;
	
	UPROPERTY(BlueprintReadOnly, Category="EssentialData|Lean")
	float LeanY;
	
	FVector Lean;

	UPROPERTY(BlueprintReadOnly, Category="EssentialData|Lean")
	float AimYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="EssentialData|Lean")
	float AimPitch;
	
#pragma endregion

#pragma region Locomotion
	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	ELocomotionState LocomotionState;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	ELocomotionState PrevLocomotionState;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float TimeInLocomotionState;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float PlayRate;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool PlayStartAnim;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool PlayGaitTransitionAnim;

#pragma region AnimationData
	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	UAnimSequence* StopAnim;

	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	UAnimSequence* WalkStartAnim;

	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	UAnimSequence* RunStartAnim;

	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	UAnimSequence* RunToWalkAnim;

	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	UAnimSequence* WalkToRunAnim;

	UPROPERTY(BlueprintReadOnly, Category="AnimationData")
	float AnimStartTime;
#pragma endregion

#pragma region Curves
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataSpeedCurveName = "MoveData_Speed";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataFootPhaseCurveName = "MoveData_FootPhase";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataRotationBlendName = "MoveData_RotationBlend";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataLeanXName = "MoveData_LeanX";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataLeanYName = "MoveData_LeanY";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves")
	float MoveDataSpeedMinClampValue = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves")
	float MoveDataSpeedMaxClampValue = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves")
	float MoveDataLeftFootPhaseLimit = 0.5f;
#pragma endregion
#pragma endregion


#pragma region Rotation
	UPROPERTY(BlueprintReadOnly, Category="Rotation")
	FRotator StartRotation;

	UPROPERTY(BlueprintReadOnly, Category="Rotation")
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadOnly, Category="Rotation")
	FRotator TargetRotationSmoothed;

	UPROPERTY(BlueprintReadOnly, Category="Rotation")
	float StartAngle;
#pragma endregion

private:
#pragma region EssentialData
	UPROPERTY(EditDefaultsOnly, Category="EssentialData")
	float InputVectorRotationRateInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="EssentialData")
	float LeanInterpSpeed = 4.f;
#pragma endregion

#pragma region Locomotion
	UPROPERTY(EditDefaultsOnly, Category="Locomotion")
	float MinTimeInLocomotionState = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category="Locomotion")
	float MinTimeGaitTransitionAnim = 1.f;

#pragma region Run
	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Run")
	float RunMinCurrentSpeed = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Run")
	float RunMinMaxSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Run")
	float RunMinInputAcceleration = 0.5f;
#pragma endregion

#pragma region Walk
	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Walk")
	float WalkMinCurrentSpeed = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Walk")
	float WalkMinMaxSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Walk")
	float WalkMinInputAcceleration = 0.01f;
#pragma endregion

#pragma region Start
	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Start")
	float MaxSpeedForPlayingStartAnim = 150.f;
#pragma endregion

#pragma region Stop
	UPROPERTY(EditDefaultsOnly, Category="Locomotion|Stop")
	float RunStopSpeedLimit = 200.f;
#pragma endregion

#pragma region Animations
	UPROPERTY(EditDefaultsOnly, Category="Animations|Stop")
	UAnimSequence* WalkStopAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Stop")
	float WalkAnimStartTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Stop")
	UAnimSequence* RunStopAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Stop")
	float RunAnimStartTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	UAnimSequence* WalkStart90LAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	float WalkStart90LAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	UAnimSequence* WalkStart180LAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	float WalkStart180LAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	UAnimSequence* WalkStart90RAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	float WalkStart90RAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	UAnimSequence* WalkStart180RAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	float WalkStart180RAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	UAnimSequence* WalkStartFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Walk")
	float WalkStartFAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	UAnimSequence* RunStart90LAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	float RunStart90LAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	UAnimSequence* RunStart180LAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	float RunStart180LAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	UAnimSequence* RunStart90RAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	float RunStart90RAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	UAnimSequence* RunStart180RAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	float RunStart180RAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	UAnimSequence* RunStartFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Start|Run")
	float RunStartFAnimTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	UAnimSequence* WalkToRunLFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	float WalkToRunLFTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	UAnimSequence* WalkToRunRFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	float WalkToRunRFTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	UAnimSequence* RunToWalkLFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	float RunToWalkLFTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	UAnimSequence* RunToWalkRFAnim;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Transition")
	float RunToWalkRFTime = 0.f;

#pragma endregion
#pragma endregion

private:
#pragma region HelperFunctions
	FORCEINLINE bool IsMovementWithinThresholds(float MinCurrentSpeed, float MinMaxSpeed,
	                                            float MinInputAcceleration) const;

	FORCEINLINE void TrackLocomotionState(ELocomotionState TracedState,
	                                      bool& EnterFlag,
	                                      void (UPlayerAnimInstance::*OnEnterCallback)(),
	                                      void (UPlayerAnimInstance::*OnExitCallback)(),
	                                      void (UPlayerAnimInstance::*WhileTrueCallback)(),
	                                      void (UPlayerAnimInstance::*WhileFalseCallback)());

	FORCEINLINE void UpdateStartAnim(UAnimSequence*& FinishAnim,
							  UAnimSequence* Start90LAnim, float Start90LAnimTime,
							  UAnimSequence* Start180LAnim, float Start180LAnimTime,
							  UAnimSequence* Start90RAnim, float Start90RAnimTime,
							  UAnimSequence* Start180RAnim, float Start180RAnimTime,
							  UAnimSequence* StartFAnim, float StartFAnimTime);

	FORCEINLINE void UpdateTransitionAnim(UAnimSequence*& FinishAnim,
	                          UAnimSequence* TransitionLFAnim, float AnimLFStartTime,
	                          UAnimSequence* TransitionRFAnim, float AnimRFStartTime);

	FORCEINLINE float CalculateConstRotationRate() const;
#pragma endregion

private:
#pragma region TrackLocomotionState
	OnEntryFlags EntryFlags;

#pragma region IdleCallbacks
	void OnEntryIdle();
	void OnExitIdle();
	void WhileTrueIdle();
	void WhileFalseIdle();
#pragma endregion

#pragma region WalkCallbacks
	void OnEntryWalk();
	void OnExitWalk();
	void WhileTrueWalk();
	void WhileFalseWalk();
#pragma endregion

#pragma region RunCallbacks
	void OnEntryRun();
	void OnExitRun();
	void WhileTrueRun();
	void WhileFalseRun();
#pragma endregion

#pragma region CrouchCallbacks
	void OnEntryCrouch();
	void OnExitCrouch();
	void WhileTrueCrouch();
	void WhileFalseCrouch();
#pragma endregion

#pragma region JumpCallbacks
	void OnEntryJump();
	void OnExitJump();
	void WhileTrueJump();
	void WhileFalseJump();
#pragma endregion

#pragma endregion
};
