; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --check-globals all --version 5
; RUN: opt -S -mtriple=amdgcn-- -passes=amdgpu-lower-module-lds < %s | FileCheck %s

; This is an extension and should be rejected by the front-end in most cases.
; If it goes through, lower it as dynlds.

@Var0 = linkonce_odr hidden local_unnamed_addr addrspace(3) global [0 x float] poison

;.
; CHECK: @llvm.amdgcn.kernelA.dynlds = external addrspace(3) global [0 x i8], align 4, !absolute_symbol [[META0:![0-9]+]]
; CHECK: @llvm.amdgcn.dynlds.offset.table = internal addrspace(4) constant [1 x i32] [i32 ptrtoint (ptr addrspace(3) @llvm.amdgcn.kernelA.dynlds to i32)]
;.
define void @fn(float %val, i32 %idx) #0 {
; CHECK-LABEL: define void @fn(
; CHECK-SAME: float [[VAL:%.*]], i32 [[IDX:%.*]]) {
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.amdgcn.lds.kernel.id()
; CHECK-NEXT:    [[VAR0:%.*]] = getelementptr inbounds [1 x i32], ptr addrspace(4) @llvm.amdgcn.dynlds.offset.table, i32 0, i32 [[TMP1]]
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr addrspace(4) [[VAR0]], align 4
; CHECK-NEXT:    [[VAR01:%.*]] = inttoptr i32 [[TMP2]] to ptr addrspace(3)
; CHECK-NEXT:    [[PTR:%.*]] = getelementptr i32, ptr addrspace(3) [[VAR01]], i32 [[IDX]]
; CHECK-NEXT:    store float [[VAL]], ptr addrspace(3) [[PTR]], align 4
; CHECK-NEXT:    ret void
;
  %ptr = getelementptr i32, ptr addrspace(3) @Var0, i32 %idx
  store float %val, ptr addrspace(3) %ptr
  ret void
}

define amdgpu_kernel void @kernelA(float %val, i32 %idx) #0 {
; CHECK-LABEL: define amdgpu_kernel void @kernelA(
; CHECK-SAME: float [[VAL:%.*]], i32 [[IDX:%.*]]) !llvm.amdgcn.lds.kernel.id [[META1:![0-9]+]] {
; CHECK-NEXT:    call void @llvm.donothing() [ "ExplicitUse"(ptr addrspace(3) @llvm.amdgcn.kernelA.dynlds) ]
; CHECK-NEXT:    tail call void @fn(float [[VAL]], i32 [[IDX]])
; CHECK-NEXT:    ret void
;
  tail call void @fn(float %val, i32 %idx)
  ret void
}

attributes #0 = { "amdgpu-no-lds-kernel-id" }

;.
; CHECK: attributes #[[ATTR0:[0-9]+]] = { nocallback nofree nosync nounwind willreturn memory(none) }
; CHECK: attributes #[[ATTR1:[0-9]+]] = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
;.
; CHECK: [[META0]] = !{i32 0, i32 1}
; CHECK: [[META1]] = !{i32 0}
;.
