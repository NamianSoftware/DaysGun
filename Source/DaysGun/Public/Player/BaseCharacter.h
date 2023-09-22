// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseCharacter.generated.h"


/* Forward declarations */
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


UCLASS()
class DAYSGUN_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Components

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

#pragma region Meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BackpackMesh;
#pragma endregion
#pragma endregion

public:
	ABaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


#pragma region CharacterSettings

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Speed", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 175.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Speed", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 300.f;

private:
	void SetupCharacterSettings();
#pragma endregion

#pragma region Input

private:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for sptring input */
	void RunStarted(const FInputActionValue& Value);
	void RunFinished(const FInputActionValue& Value);
#pragma endregion

#pragma region SmoothSpeedTransition
private:
	UPROPERTY(EditDefaultsOnly, Category="Settings|Speed", meta = (AllowPrivateAccess = "true"))
	float MaxSpeedTransitionInterp = 4.f;
	
	float TargetMaxSpeed;

private:
	void UpdateMaxSpeed(float DeltaSeconds);
#pragma endregion
};
