// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix=LINUX
// RUN: %clang_cc1 -triple x86_64-windows-pc -emit-llvm %s -o - | FileCheck %s --check-prefix=WINDOWS

int __attribute__((target("sse4.2"))) foo(void) { return 0; }
int __attribute__((target("arch=sandybridge"))) foo(void);
int __attribute__((target("arch=ivybridge"))) foo(void) {return 1;}
int __attribute__((target("default"))) foo(void) { return 2; }

int bar() {
  return foo();
}

inline int __attribute__((target("sse4.2"))) foo_inline(void) { return 0; }
inline int __attribute__((target("arch=sandybridge"))) foo_inline(void);
inline int __attribute__((target("arch=ivybridge"))) foo_inline(void) {return 1;}
inline int __attribute__((target("default"))) foo_inline(void) { return 2; }

int bar2() {
  return foo_inline();
}

inline __attribute__((target("default"))) void foo_decls(void);
inline __attribute__((target("sse4.2"))) void foo_decls(void);
void bar3() {
  foo_decls();
}
inline __attribute__((target("default"))) void foo_decls(void) {}
inline __attribute__((target("sse4.2"))) void foo_decls(void) {}

inline __attribute__((target("default"))) void foo_multi(int i, double d) {}
inline __attribute__((target("avx,sse4.2"))) void foo_multi(int i, double d) {}
inline __attribute__((target("sse4.2,fma4"))) void foo_multi(int i, double d) {}
inline __attribute__((target("arch=ivybridge,fma4,sse4.2"))) void foo_multi(int i, double d) {}
void bar4() {
  foo_multi(1, 5.0);
}

// LINUX: @foo.ifunc = ifunc i32 (), i32 ()* ()* @foo.resolver
// LINUX: @foo_inline.ifunc = ifunc i32 (), i32 ()* ()* @foo_inline.resolver
// LINUX: @foo_decls.ifunc = ifunc void (), void ()* ()* @foo_decls.resolver
// LINUX: @foo_multi.ifunc = ifunc void (i32, double), void (i32, double)* ()* @foo_multi.resolver

// LINUX: define i32 @foo.sse4.2()
// LINUX: ret i32 0
// LINUX: define i32 @foo.arch_ivybridge()
// LINUX: ret i32 1
// LINUX: define i32 @foo()
// LINUX: ret i32 2
// LINUX: define i32 @bar()
// LINUX: call i32 @foo.ifunc()

// WINDOWS: define dso_local i32 @foo.sse4.2()
// WINDOWS: ret i32 0
// WINDOWS: define dso_local i32 @foo.arch_ivybridge()
// WINDOWS: ret i32 1
// WINDOWS: define dso_local i32 @foo()
// WINDOWS: ret i32 2
// WINDOWS: define dso_local i32 @bar()
// WINDOWS: call i32 @foo.resolver()

// LINUX: define i32 ()* @foo.resolver() comdat
// LINUX: call void @__cpu_indicator_init()
// LINUX: ret i32 ()* @foo.arch_sandybridge
// LINUX: ret i32 ()* @foo.arch_ivybridge
// LINUX: ret i32 ()* @foo.sse4.2
// LINUX: ret i32 ()* @foo

// WINDOWS: define dso_local i32 @foo.resolver() comdat
// WINDOWS: call void @__cpu_indicator_init()
// WINDOWS: call i32 @foo.arch_sandybridge
// WINDOWS: call i32 @foo.arch_ivybridge
// WINDOWS: call i32 @foo.sse4.2
// WINDOWS: call i32 @foo

// LINUX: define linkonce i32 @foo_inline.arch_ivybridge()
// LINUX: ret i32 1
// LINUX: define linkonce i32 @foo_inline()
// LINUX: ret i32 2

// WINDOWS: define linkonce_odr dso_local i32 @foo_inline.arch_ivybridge()
// WINDOWS: ret i32 1
// WINDOWS: define linkonce_odr dso_local i32 @foo_inline()
// WINDOWS: ret i32 2

// LINUX: define i32 @bar2()
// LINUX: call i32 @foo_inline.ifunc()

// WINDOWS: define dso_local i32 @bar2()
// WINDOWS: call i32 @foo_inline.resolver()

// LINUX: define i32 ()* @foo_inline.resolver() comdat
// LINUX: call void @__cpu_indicator_init()
// LINUX: ret i32 ()* @foo_inline.arch_sandybridge
// LINUX: ret i32 ()* @foo_inline.arch_ivybridge
// LINUX: ret i32 ()* @foo_inline.sse4.2
// LINUX: ret i32 ()* @foo_inline

// WINDOWS: define dso_local i32 @foo_inline.resolver() comdat
// WINDOWS: call void @__cpu_indicator_init()
// WINDOWS: call i32 @foo_inline.arch_sandybridge
// WINDOWS: call i32 @foo_inline.arch_ivybridge
// WINDOWS: call i32 @foo_inline.sse4.2
// WINDOWS: call i32 @foo_inline

// LINUX: define void @bar3()
// LINUX: call void @foo_decls.ifunc()

// WINDOWS: define dso_local void @bar3()
// WINDOWS: call void @foo_decls.resolver()

// LINUX: define void ()* @foo_decls.resolver() comdat
// LINUX: ret void ()* @foo_decls.sse4.2
// LINUX: ret void ()* @foo_decls

// WINDOWS: define dso_local void @foo_decls.resolver() comdat
// WINDOWS: call void @foo_decls.sse4.2
// Windows: call void @foo_decls

// LINUX: define linkonce void @foo_decls()
// LINUX: define linkonce void @foo_decls.sse4.2()

// WINDOWS: define linkonce_odr dso_local void @foo_decls()
// WINDOWS: define linkonce_odr dso_local void @foo_decls.sse4.2()

// LINUX: define linkonce void @foo_multi(i32 %{{[^,]+}}, double %{{[^\)]+}})
// LINUX: define linkonce void @foo_multi.avx_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})
// LINUX: define linkonce void @foo_multi.fma4_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})
// LINUX: define linkonce void @foo_multi.arch_ivybridge_fma4_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})

// WINDOWS: define linkonce_odr dso_local void @foo_multi(i32 %{{[^,]+}}, double %{{[^\)]+}})
// WINDOWS: define linkonce_odr dso_local void @foo_multi.avx_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})
// WINDOWS: define linkonce_odr dso_local void @foo_multi.fma4_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})
// WINDOWS: define linkonce_odr dso_local void @foo_multi.arch_ivybridge_fma4_sse4.2(i32 %{{[^,]+}}, double %{{[^\)]+}})

// LINUX: define void @bar4()
// LINUX: call void @foo_multi.ifunc(i32 1, double 5.{{[0+e]*}})

// WINDOWS: define dso_local void @bar4()
// WINDOWS: call void @foo_multi.resolver(i32 1, double 5.{{[0+e]*}})

// LINUX: define void (i32, double)* @foo_multi.resolver() comdat
// LINUX: and i32 %{{.*}}, 4352
// LINUX: icmp eq i32 %{{.*}}, 4352
// LINUX: ret void (i32, double)* @foo_multi.fma4_sse4.2
// LINUX: icmp eq i32 %{{.*}}, 12
// LINUX: and i32 %{{.*}}, 4352
// LINUX: icmp eq i32 %{{.*}}, 4352
// LINUX: ret void (i32, double)* @foo_multi.arch_ivybridge_fma4_sse4.2
// LINUX: and i32 %{{.*}}, 768
// LINUX: icmp eq i32 %{{.*}}, 768
// LINUX: ret void (i32, double)* @foo_multi.avx_sse4.2
// LINUX: ret void (i32, double)* @foo_multi

// WINDOWS: define dso_local void @foo_multi.resolver(i32, double) comdat
// WINDOWS: and i32 %{{.*}}, 4352
// WINDOWS: icmp eq i32 %{{.*}}, 4352
// WINDOWS: call void @foo_multi.fma4_sse4.2(i32 %0, double %1)
// WINDOWS-NEXT: ret void
// WINDOWS: icmp eq i32 %{{.*}}, 12
// WINDOWS: and i32 %{{.*}}, 4352
// WINDOWS: icmp eq i32 %{{.*}}, 4352
// WINDOWS: call void @foo_multi.arch_ivybridge_fma4_sse4.2(i32 %0, double %1)
// WINDOWS-NEXT: ret void
// WINDOWS: and i32 %{{.*}}, 768
// WINDOWS: icmp eq i32 %{{.*}}, 768
// WINDOWS: call void @foo_multi.avx_sse4.2(i32 %0, double %1)
// WINDOWS-NEXT: ret void
// WINDOWS: call void @foo_multi(i32 %0, double %1)
// WINDOWS-NEXT: ret void

// LINUX: declare i32 @foo.arch_sandybridge()

// WINDOWS: declare dso_local i32 @foo.arch_sandybridge()

// LINUX: define linkonce i32 @foo_inline.sse4.2()
// LINUX: ret i32 0

// WINDOWS: define linkonce_odr dso_local i32 @foo_inline.sse4.2()
// WINDOWS: ret i32 0

// LINUX: declare i32 @foo_inline.arch_sandybridge()

// WINDOWS: declare dso_local i32 @foo_inline.arch_sandybridge()

