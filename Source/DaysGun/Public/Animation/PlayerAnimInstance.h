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

	void DetermineGroundLocomotionState();

	void UpdateStop();
	void UpdateOnWalkEntry();
	void UpdateOnRunToWalk();
	void UpdateLocomotionValues();

	void CycleRotationBehavior();
	void StartRotationBehavior();

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

	UPROPERTY(BlueprintReadOnly, Category="EssentialData")
	FVector Lean;
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

#pragma region Animations
	UPROPERTY(BlueprintReadOnly, Category="Locomotion|Anims")
	UAnimSequence* StopAnim;
	
	UPROPERTY(BlueprintReadOnly, Category="Locomotion|Anims")
	float AnimStartTime;
#pragma endregion

#pragma region Curves
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	FName MoveDataSpeedCurveName = "MoveData_Speed";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	float MoveDataSpeedMinClampValue = 50.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Curves|Names")
	float MoveDataSpeedMaxClampValue = 1000.f;
#pragma endregion
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
	UPROPERTY(EditDefaultsOnly, Category="Anmimations|Stop")
	UAnimSequence* WalkStopAnim;
	
	UPROPERTY(EditDefaultsOnly, Category="Anmimations|Stop")
	float WalkAnimStartTime = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Anmimations|Stop")
	UAnimSequence* RunStopAnim;
	
	UPROPERTY(EditDefaultsOnly, Category="Anmimations|Stop")
	float RunAnimStartTime = 0.f;
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
