# This file is automatically generated.  DO NOT EDIT!
# Generated from: 	NetBSD: mknative-gcc,v 1.22 2006/06/25 03:06:15 mrg Exp 
# Generated from: NetBSD: mknative.common,v 1.9 2007/02/05 18:26:01 apb Exp 
#
G_BUILD_EARLY_SUPPORT=gensupport.o dummy-conditions.o
G_BUILD_ERRORS=build-errors.o
G_BUILD_PRINT=build-print-rtl.o
G_BUILD_RTL=build-rtl.o read-rtl.o build-ggc-none.o min-insn-modes.o
G_BUILD_SUPPORT=gensupport.o insn-conditions.o
G_BUILD_VARRAY=build-varray.o
G_ALL_CFLAGS=  -g -DIN_GCC   -W -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -pedantic -Wno-long-long -Wno-variadic-macros -Wold-style-definition -Wmissing-format-attribute    -DHAVE_CONFIG_H
G_ALL_CPPFLAGS=-I. -I. -I${GNUHOSTDIST}/gcc -I${GNUHOSTDIST}/gcc/. -I${GNUHOSTDIST}/gcc/../include -I${GNUHOSTDIST}/gcc/../libcpp/include    
G_C_AND_OBJC_OBJS=attribs.o c-errors.o c-lex.o c-pragma.o c-decl.o c-typeck.o c-convert.o c-aux-info.o c-common.o c-opts.o c-format.o c-semantics.o c-incpath.o cppdefault.o c-ppoutput.o c-cppbuiltin.o prefix.o c-objc-common.o c-dump.o c-pch.o c-parser.o  c-gimplify.o tree-mudflap.o c-pretty-print.o
G_C_OBJS=c-lang.o stub-objc.o attribs.o c-errors.o c-lex.o c-pragma.o c-decl.o c-typeck.o c-convert.o c-aux-info.o c-common.o c-opts.o c-format.o c-semantics.o c-incpath.o cppdefault.o c-ppoutput.o c-cppbuiltin.o prefix.o c-objc-common.o c-dump.o c-pch.o c-parser.o  c-gimplify.o tree-mudflap.o c-pretty-print.o
G_CCCP_OBJS=
G_GCOV_OBJS=gcov.o intl.o version.o errors.o
G_PROTO_OBJS=intl.o version.o cppdefault.o errors.o
G_INCLUDES=-I. -I. -I${GNUHOSTDIST}/gcc -I${GNUHOSTDIST}/gcc/. -I${GNUHOSTDIST}/gcc/../include -I${GNUHOSTDIST}/gcc/../libcpp/include 
G_md_file=${GNUHOSTDIST}/gcc/config/arm/arm.md
G_OBJC_OBJS=objc/objc-lang.o objc/objc-act.o
G_OBJS=tree-chrec.o tree-scalar-evolution.o tree-data-ref.o tree-cfg.o tree-dfa.o tree-eh.o tree-ssa.o tree-optimize.o tree-gimple.o gimplify.o tree-pretty-print.o tree-into-ssa.o tree-outof-ssa.o tree-ssa-ccp.o tree-vn.o tree-ssa-uncprop.o tree-ssa-dce.o  tree-ssa-copy.o tree-nrv.o tree-ssa-copyrename.o tree-ssa-pre.o tree-ssa-live.o tree-ssa-operands.o tree-ssa-alias.o tree-ssa-phiopt.o tree-ssa-forwprop.o tree-nested.o tree-ssa-dse.o tree-ssa-dom.o domwalk.o tree-tailcall.o gimple-low.o tree-iterator.o tree-phinodes.o tree-ssanames.o tree-sra.o tree-complex.o tree-vect-generic.o tree-ssa-loop.o tree-ssa-loop-niter.o tree-ssa-loop-manip.o tree-ssa-threadupdate.o tree-vectorizer.o tree-vect-analyze.o tree-vect-transform.o tree-ssa-loop-ivcanon.o tree-ssa-propagate.o tree-ssa-address.o tree-ssa-math-opts.o tree-ssa-loop-ivopts.o tree-if-conv.o tree-ssa-loop-unswitch.o alias.o bb-reorder.o bitmap.o builtins.o caller-save.o calls.o cfg.o cfganal.o cfgbuild.o cfgcleanup.o cfglayout.o cfgloop.o cfgloopanal.o cfgloopmanip.o loop-init.o loop-unswitch.o loop-unroll.o cfgrtl.o combine.o conflict.o convert.o coverage.o cse.o cselib.o dbxout.o ddg.o tree-ssa-loop-ch.o loop-invariant.o tree-ssa-loop-im.o debug.o df.o diagnostic.o dojump.o dominance.o loop-doloop.o dwarf2asm.o dwarf2out.o emit-rtl.o except.o explow.o loop-iv.o expmed.o expr.o final.o flow.o fold-const.o function.o gcse.o genrtl.o ggc-common.o global.o graph.o gtype-desc.o haifa-sched.o hooks.o ifcvt.o insn-attrtab.o insn-emit.o insn-modes.o insn-extract.o insn-opinit.o insn-output.o insn-peep.o insn-recog.o integrate.o intl.o jump.o  langhooks.o lcm.o lists.o local-alloc.o loop.o mode-switching.o modulo-sched.o optabs.o options.o opts.o params.o postreload.o postreload-gcse.o predict.o insn-preds.o pointer-set.o print-rtl.o print-tree.o profile.o value-prof.o var-tracking.o real.o recog.o reg-stack.o regclass.o regmove.o regrename.o reload.o reload1.o reorg.o resource.o rtl.o rtlanal.o rtl-error.o sbitmap.o sched-deps.o sched-ebb.o sched-rgn.o sched-vis.o sdbout.o simplify-rtx.o sreal.o stmt.o stor-layout.o stringpool.o targhooks.o timevar.o toplev.o tracer.o tree.o tree-dump.o varasm.o varray.o vec.o version.o vmsdbgout.o xcoffout.o alloc-pool.o et-forest.o cfghooks.o bt-load.o pretty-print.o ggc-page.o web.o passes.o tree-profile.o rtlhooks.o cfgexpand.o lambda-mat.o lambda-trans.o	lambda-code.o tree-loop-linear.o tree-ssa-sink.o tree-vrp.o tree-stdarg.o tree-cfgcleanup.o tree-ssa-reassoc.o tree-ssa-structalias.o tree-object-size.o arm.o  host-default.o tree-inline.o cgraph.o cgraphunit.o tree-nomudflap.o ipa.o ipa-inline.o ipa-utils.o ipa-reference.o ipa-pure-const.o ipa-type-escape.o ipa-prop.o ipa-cp.o
G_out_file=${GNUHOSTDIST}/gcc/config/arm/arm.c
G_version=4.1.3
G_BUILD_PREFIX=
G_RTL_H=rtl.h rtl.def machmode.h mode-classes.def insn-modes.h reg-notes.def insn-notes.def input.h real.h statistics.h genrtl.h
G_TREE_H=tree.h tree.def machmode.h mode-classes.def insn-modes.h tree-check.h builtins.def input.h statistics.h vec.h treestruct.def
G_BASIC_BLOCK_H=basic-block.h bitmap.h sbitmap.h varray.h ${GNUHOSTDIST}/gcc/../include/partition.h hard-reg-set.h cfghooks.h ${GNUHOSTDIST}/gcc/../include/obstack.h
G_GCC_H=gcc.h version.h
G_GTFILES_SRCDIR=${GNUHOSTDIST}/gcc
G_GTFILES_FILES_FILES=${GNUHOSTDIST}/gcc/cp/rtti.c  ${GNUHOSTDIST}/gcc/cp/mangle.c  ${GNUHOSTDIST}/gcc/cp/name-lookup.h  ${GNUHOSTDIST}/gcc/cp/name-lookup.c  ${GNUHOSTDIST}/gcc/cp/cp-tree.h  ${GNUHOSTDIST}/gcc/cp/decl.h  ${GNUHOSTDIST}/gcc/cp/call.c  ${GNUHOSTDIST}/gcc/cp/decl.c  ${GNUHOSTDIST}/gcc/cp/decl2.c  ${GNUHOSTDIST}/gcc/cp/pt.c  ${GNUHOSTDIST}/gcc/cp/repo.c  ${GNUHOSTDIST}/gcc/cp/semantics.c  ${GNUHOSTDIST}/gcc/cp/tree.c  ${GNUHOSTDIST}/gcc/cp/parser.c  ${GNUHOSTDIST}/gcc/cp/method.c  ${GNUHOSTDIST}/gcc/cp/typeck2.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-lex.c  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/cp/class.c  ${GNUHOSTDIST}/gcc/cp/cp-objcp-common.c  ${GNUHOSTDIST}/gcc/objc/objc-act.h  ${GNUHOSTDIST}/gcc/c-parser.c  ${GNUHOSTDIST}/gcc/c-tree.h  ${GNUHOSTDIST}/gcc/c-decl.c  ${GNUHOSTDIST}/gcc/c-objc-common.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/objc/objc-act.c  ${GNUHOSTDIST}/gcc/c-lang.c  ${GNUHOSTDIST}/gcc/c-tree.h  ${GNUHOSTDIST}/gcc/c-decl.c  ${GNUHOSTDIST}/gcc/c-common.c  ${GNUHOSTDIST}/gcc/c-common.h  ${GNUHOSTDIST}/gcc/c-pragma.c  ${GNUHOSTDIST}/gcc/c-objc-common.c  ${GNUHOSTDIST}/gcc/c-parser.c 
G_GTFILES_FILES_LANGS=cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  cp  objc  objc  objc  objc  objc  objc  objc  objc  objc  c  c  c  c  c  c  c  c 
G_GTFILES=${GNUHOSTDIST}/gcc/input.h ${GNUHOSTDIST}/gcc/coretypes.h ${GNUHOSTDIST}/gcc/../libcpp/include/line-map.h ${GNUHOSTDIST}/gcc/../libcpp/include/cpplib.h ${GNUHOSTDIST}/gcc/../libcpp/include/cpp-id-data.h auto-host.h ${GNUHOSTDIST}/gcc/../include/ansidecl.h options.h ${GNUHOSTDIST}/gcc/config/dbxelf.h ${GNUHOSTDIST}/gcc/config/elfos.h ${GNUHOSTDIST}/gcc/config/netbsd.h ${GNUHOSTDIST}/gcc/config/netbsd-elf.h ${GNUHOSTDIST}/gcc/config/arm/elf.h ${GNUHOSTDIST}/gcc/config/arm/aout.h ${GNUHOSTDIST}/gcc/config/arm/arm.h ${GNUHOSTDIST}/gcc/config/arm/netbsd-elf.h ${GNUHOSTDIST}/gcc/defaults.h ${GNUHOSTDIST}/gcc/../include/hashtab.h ${GNUHOSTDIST}/gcc/../include/splay-tree.h ${GNUHOSTDIST}/gcc/bitmap.h ${GNUHOSTDIST}/gcc/coverage.c ${GNUHOSTDIST}/gcc/function.h ${GNUHOSTDIST}/gcc/rtl.h ${GNUHOSTDIST}/gcc/optabs.h ${GNUHOSTDIST}/gcc/tree.h ${GNUHOSTDIST}/gcc/libfuncs.h ${GNUHOSTDIST}/gcc/../libcpp/include/symtab.h ${GNUHOSTDIST}/gcc/real.h ${GNUHOSTDIST}/gcc/varray.h ${GNUHOSTDIST}/gcc/insn-addr.h ${GNUHOSTDIST}/gcc/hwint.h ${GNUHOSTDIST}/gcc/ipa-reference.h ${GNUHOSTDIST}/gcc/cselib.h ${GNUHOSTDIST}/gcc/basic-block.h  ${GNUHOSTDIST}/gcc/cgraph.h ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-tree.h ${GNUHOSTDIST}/gcc/reload.h ${GNUHOSTDIST}/gcc/alias.c ${GNUHOSTDIST}/gcc/bitmap.c ${GNUHOSTDIST}/gcc/cselib.c ${GNUHOSTDIST}/gcc/cgraph.c ${GNUHOSTDIST}/gcc/ipa-prop.c ${GNUHOSTDIST}/gcc/ipa-cp.c ${GNUHOSTDIST}/gcc/cgraphunit.c ${GNUHOSTDIST}/gcc/dbxout.c ${GNUHOSTDIST}/gcc/dwarf2out.c ${GNUHOSTDIST}/gcc/dwarf2asm.c ${GNUHOSTDIST}/gcc/dojump.c ${GNUHOSTDIST}/gcc/tree-profile.c ${GNUHOSTDIST}/gcc/emit-rtl.c ${GNUHOSTDIST}/gcc/except.c ${GNUHOSTDIST}/gcc/explow.c ${GNUHOSTDIST}/gcc/expr.c ${GNUHOSTDIST}/gcc/function.c ${GNUHOSTDIST}/gcc/except.h ${GNUHOSTDIST}/gcc/gcse.c ${GNUHOSTDIST}/gcc/integrate.c ${GNUHOSTDIST}/gcc/lists.c ${GNUHOSTDIST}/gcc/optabs.c ${GNUHOSTDIST}/gcc/profile.c ${GNUHOSTDIST}/gcc/regclass.c ${GNUHOSTDIST}/gcc/reg-stack.c ${GNUHOSTDIST}/gcc/cfglayout.c ${GNUHOSTDIST}/gcc/sdbout.c ${GNUHOSTDIST}/gcc/stor-layout.c ${GNUHOSTDIST}/gcc/stringpool.c ${GNUHOSTDIST}/gcc/tree.c ${GNUHOSTDIST}/gcc/varasm.c ${GNUHOSTDIST}/gcc/tree-mudflap.c ${GNUHOSTDIST}/gcc/tree-flow.h ${GNUHOSTDIST}/gcc/c-objc-common.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-parser.c ${GNUHOSTDIST}/gcc/tree-ssanames.c ${GNUHOSTDIST}/gcc/tree-eh.c ${GNUHOSTDIST}/gcc/tree-ssa-address.c ${GNUHOSTDIST}/gcc/tree-phinodes.c ${GNUHOSTDIST}/gcc/tree-cfg.c ${GNUHOSTDIST}/gcc/tree-dfa.c ${GNUHOSTDIST}/gcc/tree-ssa-propagate.c ${GNUHOSTDIST}/gcc/tree-iterator.c ${GNUHOSTDIST}/gcc/gimplify.c ${GNUHOSTDIST}/gcc/tree-chrec.h ${GNUHOSTDIST}/gcc/tree-vect-generic.c ${GNUHOSTDIST}/gcc/tree-ssa-operands.h ${GNUHOSTDIST}/gcc/tree-ssa-operands.c ${GNUHOSTDIST}/gcc/tree-profile.c ${GNUHOSTDIST}/gcc/tree-nested.c ${GNUHOSTDIST}/gcc/ipa-reference.c ${GNUHOSTDIST}/gcc/tree-ssa-structalias.h ${GNUHOSTDIST}/gcc/tree-ssa-structalias.c ${GNUHOSTDIST}/gcc/targhooks.c ${GNUHOSTDIST}/gcc/config/arm/arm.c ${GNUHOSTDIST}/gcc/cp/rtti.c ${GNUHOSTDIST}/gcc/cp/mangle.c ${GNUHOSTDIST}/gcc/cp/name-lookup.h ${GNUHOSTDIST}/gcc/cp/name-lookup.c ${GNUHOSTDIST}/gcc/cp/cp-tree.h ${GNUHOSTDIST}/gcc/cp/decl.h ${GNUHOSTDIST}/gcc/cp/call.c ${GNUHOSTDIST}/gcc/cp/decl.c ${GNUHOSTDIST}/gcc/cp/decl2.c ${GNUHOSTDIST}/gcc/cp/pt.c ${GNUHOSTDIST}/gcc/cp/repo.c ${GNUHOSTDIST}/gcc/cp/semantics.c ${GNUHOSTDIST}/gcc/cp/tree.c ${GNUHOSTDIST}/gcc/cp/parser.c ${GNUHOSTDIST}/gcc/cp/method.c ${GNUHOSTDIST}/gcc/cp/typeck2.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-lex.c ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/cp/class.c ${GNUHOSTDIST}/gcc/cp/cp-objcp-common.c ${GNUHOSTDIST}/gcc/objc/objc-act.h ${GNUHOSTDIST}/gcc/c-parser.c ${GNUHOSTDIST}/gcc/c-tree.h ${GNUHOSTDIST}/gcc/c-decl.c ${GNUHOSTDIST}/gcc/c-objc-common.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/objc/objc-act.c ${GNUHOSTDIST}/gcc/c-lang.c ${GNUHOSTDIST}/gcc/c-tree.h ${GNUHOSTDIST}/gcc/c-decl.c ${GNUHOSTDIST}/gcc/c-common.c ${GNUHOSTDIST}/gcc/c-common.h ${GNUHOSTDIST}/gcc/c-pragma.c ${GNUHOSTDIST}/gcc/c-objc-common.c ${GNUHOSTDIST}/gcc/c-parser.c
G_GTFILES_LANG_DIR_NAMES=cp objc
G_tm_defines=NETBSD_ENABLE_PTHREADS TARGET_ENDIAN_DEFAULT=MASK_BIG_END
G_host_xm_file=
G_host_xm_defines=
G_tm_p_file=
G_target_cpu_default=TARGET_CPU_generic
G_TM_H=tm.h      options.h ${GNUHOSTDIST}/gcc/config/dbxelf.h ${GNUHOSTDIST}/gcc/config/elfos.h ${GNUHOSTDIST}/gcc/config/netbsd.h ${GNUHOSTDIST}/gcc/config/netbsd-elf.h ${GNUHOSTDIST}/gcc/config/arm/elf.h ${GNUHOSTDIST}/gcc/config/arm/aout.h ${GNUHOSTDIST}/gcc/config/arm/arm.h ${GNUHOSTDIST}/gcc/config/arm/netbsd-elf.h ${GNUHOSTDIST}/gcc/defaults.h insn-constants.h insn-flags.h options.h
G_ALL_OPT_FILES=${GNUHOSTDIST}/gcc/c.opt ${GNUHOSTDIST}/gcc/common.opt ${GNUHOSTDIST}/gcc/config/arm/arm.opt
G_tm_file_list=options.h ${GNUHOSTDIST}/gcc/config/dbxelf.h ${GNUHOSTDIST}/gcc/config/elfos.h ${GNUHOSTDIST}/gcc/config/netbsd.h ${GNUHOSTDIST}/gcc/config/netbsd-elf.h ${GNUHOSTDIST}/gcc/config/arm/elf.h ${GNUHOSTDIST}/gcc/config/arm/aout.h ${GNUHOSTDIST}/gcc/config/arm/arm.h ${GNUHOSTDIST}/gcc/config/arm/netbsd-elf.h ${GNUHOSTDIST}/gcc/defaults.h
G_build_xm_include_list=auto-build.h ansidecl.h
G_lang_specs_files=${GNUHOSTDIST}/gcc/cp/lang-specs.h ${GNUHOSTDIST}/gcc/objc/lang-specs.h
G_tm_p_include_list=config/arm/arm-protos.h tm-preds.h
G_LIB2ADDEHDEP= unwind-dw2-fde.h unwind-dw2-fde.c
G_CXX_OBJS=cp-lang.o stub-objc.o call.o decl.o expr.o pt.o typeck2.o class.o decl2.o error.o lex.o parser.o ptree.o rtti.o typeck.o cvt.o except.o friend.o init.o method.o search.o semantics.o tree.o repo.o dump.o optimize.o mangle.o cp-objcp-common.o name-lookup.o cxx-pretty-print.o cp-gimplify.o tree-mudflap.o attribs.o c-common.o c-format.o c-pragma.o c-semantics.o c-lex.o c-dump.o  c-pretty-print.o c-opts.o c-pch.o c-incpath.o cppdefault.o c-ppoutput.o c-cppbuiltin.o prefix.o c-gimplify.o tree-inline.o
G_CXX_C_OBJS=attribs.o c-common.o c-format.o c-pragma.o c-semantics.o c-lex.o c-dump.o  c-pretty-print.o c-opts.o c-pch.o c-incpath.o cppdefault.o c-ppoutput.o c-cppbuiltin.o prefix.o c-gimplify.o tree-inline.o
G_F77_OBJS=
G_libcpp_a_OBJS=charset.o directives.o errors.o expr.o files.o identifiers.o init.o lex.o line-map.o macro.o mkdeps.o pch.o symtab.o traditional.o
G_ENABLE_SHARED=yes
G_SHLIB_LINK= -shared
G_SHLIB_MULTILIB=.
