= 脱初心者に襲いかかるC++の罠

やっとC++初心者を脱出し、もっと良いコードを目指すあなた。
そんなあなたに襲いかかるのはC++の難解な仕様の数々です。

この章では、ちょっと高級な機能に手を出し始めた脱初心者が陥る罠の数々を解説します。

== Forwarding Reference

@<b>{Forwarding Reference}とは右辺値と左辺値のオーバーロードが同時に扱えるという夢のような挙動を実現する機能です。
その特性から、Forwarding Referenceを使ったオーバーロードはあらゆる修飾に対応するのでオーバーロードに選択されやすいという事実があります。

Forwarding Referenceで引数をとる関数は意図しない呼び出しを避けるため、SFINAEで型制約をかけるべきです。
しかし、SFINAEは難しいです。
難しいので、脱初心者段階ではForwarding Referenceに型制約を書くことができないのです。

そうして次のようなコードが生まれます。

//emlist[][cpp]{
template<typename T>
void f(const std::vector<T>&) { std::cout << "vector<T>"; }

template<typename T>
void f(T&&) { std::cout << "T&&"; }
//}

この上の関数@<code>{f(const std::vector<T>&)}は引数が@<code>{const std::vector<T>}の左辺値だった場合のみに呼ばれます。
それ以外の場合はすべて下の関数@<code>{f(T&&)}が呼ばれます。

オーバーロードをする場合は型制約をつけなければ、だいたいはADLで選ばれるので邪魔になります。
ジェネリックな関数を書いてはいけません、型制約を絶対につけなければいけません。
C++14までならば、つぎのようにenable_ifのSFINAE技法を用います。

//emlist[][cpp]{

template <class T>
struct is_vec: std::false_type {};

template <class T>
struct is_vec<std::vector<T>>: std::true_type {};

// for vector
template<typename T>
f(const std::vector<T>&) { std::cout << "vector<T>"; }

// default
template<typename T>
std::enable_if_t<!std::is_same_v<is_vecstd::decay_t<T>>::value>
f(T&&) { std::cout << "T&&"; }
//}


C++17を使っていて、オーバーロードをしないのであれば、以下のように@<code>{constexpr if}を使えばよいです。

//emlist[][cpp]{
template<typename T>
void f(T&&) {
  if constexpr (is_vec<std::decay_t<T>>::value) {
    std::cout << "vector<T>";
  }
  else {
    std::cout << "other";
  }
}
//}

== 構造化束縛

構造化束縛はC++17で追加された機能です。
ペアやタプル、配列や構造体を分解して各要素を取り出す機能で、非常に便利です。

@<code>{auto [a, b] = x}のように書いたとき、@<code>{a,b}がどのような型になるのか？
これについていったい何人に説明したか、数え切れません。

まず、大切なことを言っておきます。
新しい機能を使う前に、@<code>{cppreference.com}で一度調べてください。
雰囲気で使わないでください、おねがいします。

この機能は宣言に相当する文法です、すでに宣言された変数に代入はできません。
また、型宣言は@<code>{auto}にcvr修飾がついたものしか書くことができないです。
また、@<code>{_}のようなものを書くと値が無視されるというような便利な機能もありません（代入ができないので、もちろん@<code>{std::ignore}も使えません）。
構造化束縛は入れ子にできません。

@<code>{auto const& [a, b] = x}のように書いたから@<code>{a,b}がconstになる、、、

@<b>{とはかぎりません！}

構造化束縛の型宣言@<code>{auto const&}は分解対象@<code>{x}を受け取るときの仮引数宣言だと思ってください。

@<code>{x}が配列の場合順番に添字アクセスが繰り返されるだけです。
@<code>{std::tuple_size<T>::value}が妥当な式でないクラスの場合はメンバアクセスになります。

この2つの場合は@<code>{a,b}の型は@<code>{auto const& [a, b] = x}のように書いたから@<code>{a,b}がconstになるというのは真実です。

タプルライク型の場合は参照型を持てるため話が変わります。
@<code>{std::tuple_size<T>::value}が妥当な式であるクラスの場合は@<code>{get<i>(x)}の呼び出しによって行われます。
よってタプルライク型の場合、i番目の構造化束縛の変数の型は@<code>{std::tuple_element<i, E>::type}になります。

//emlist[][cpp]{
float x{};
char  y{};
int   z{};
 
std::tuple<float&,char&&,int> tpl(x,std::move(y),z);

const auto& [a,b,c] = tpl;

// a that refers to x; decltype(a) is float&
// b that refers to y; decltype(b) is char&&
// c that refers to the 3rd element of tpl; decltype(c) is const int
//}

要素が参照型の場合、tupleの要素型がそのまま手に入ることになります。

== Two Phase Lookup (二段階名前検索)

C++には二段階名前検索という機能があります。
これは名前の検索が二段階で行われることによって直感的な名前の解決がなされます。

=== 二段階名前検索とその必要性

二段階名前検索は次のような状況で必要となります。

//emlist[][cpp]{
#include <cstdio>

void func(void*) { std::puts("The call resolves to void*") ;}

template<typename T> void g(T x) {
    func(0);
}

void func(int) { std::puts("The call resolves to int"); }

int main() {
    g(3.14);
}
//}

この@<code>{g(3.14)}、テンプレート定義の時点で、テンプレートを書いた人は既に知られている@<code>{func(void*)}を期待しているでしょう。
ここで、新たに参加したプログラマが@<code>{func(int)}を追加してしまったとしましょう。
二段階名前検索がない場合は@<code>{g(3.14)}の時点から見えているすべての関数が検索され、@<code>{func(int)}に解決してしまいます。
一回目の検索によって@<code>{g(T)}から見えている名前が検索され、さらに二回目の検索によって実体化が行われます。

この仕組みを@<b>{Two Phase Lookup (二段階名前検索)}といいます。

=== 実体化の遅延の悪用

閉区間を表す@<code>{interval<T>}型を考えます。
この型に足し算を実装します。

//emlist[][cpp]{
template <class T> struct interval { T low, up; };

template <class T>
interval<T> operator+(interval<T> lhs, interval<T> rhs)
    { return { lhs.low + rhs.low, lhs.up + rhs.up }; }
//}

数値@<m>{x}は一点区間@<m>{[x, x]}と考えることができます。
したがって、@<code>{interval<T> + T}や@<code>{T + interval<T>}も作りたいと思うでしょう。

//emlist[][cpp]{
template <class T>
interval<T> operator+(T lhs, interval<T> rhs)
    { return { lhs + rhs, lhs.up + rhs.up }; }
template <class T>
interval<T> operator+(interval<T> lhs, T rhs)
    { return { lhs.low + rhs, lhs.up + rhs }; }
//}

ここで、一旦深呼吸をしましょう。
C++では組み込みの演算で@<code>{1 + 1.0}は許可されています。

この挙動を少し拡張して、@<code>{interval<T> + U}や@<code>{U + interval<T>}を
@<code>{interval<T>}（結果の型はinterval型に合わせる）ようにしたいと思います。

二段階名前検索を悪用すればややこしいメタプログラミングは必要ではありません。
とはいえ、多少の準備が必要です。

まず、以下のように@<code>{interval}に依存型名を追加します:

//emlist[][cpp]{
template <class T> struct interval {
    using value_type = T;
    T low, up;
};
//}

@<code>{interval<T>::value_type}のようなものを@<b>{依存名(依存型名)}といいます。
@<code>{T}がテンプレートの場合、@<code>{value_type}は@<code>{interval<T>}に依存していて、@<code>{interval<T>}が決定すると導出されます。
ただし、@<code>{interval<double>::value_type}は依存名ではありません、@<code>{interval<double>}から即座に@<code>{value_type}が@<code>{double}とわかるためです。

//emlist[][cpp]{
template <class T> struct interval {
    using value_type = T;
    T low, up;
};
//}

続いて、足し算を修正します。

//emlist[][cpp]{
template <class T>
interval<T> operator+(interval<T> lhs, interval<T> rhs)
    { return { lhs.low + rhs.low, lhs.up + rhs.up }; }

template <class T>
interval<T> operator+(typename interval<T>::value_type lhs, interval<T> rhs)
    { return { lhs + rhs, lhs.up + rhs.up }; }
template <class T>
interval<T> operator+(interval<T> lhs, typename interval<T>::value_type rhs)
    { return { lhs.low + rhs, lhs.up + rhs }; }
//}

これは@<code>{typename interval<T>::value_type}が依存名であり、関数テンプレートの実引数からの型推論の対象ではないことを利用しています。
次のコードを考えます:

//emlist[][cpp]{
  interval<int> itv{1, 2};
  auto res = itv + 1.0;
//}

まず、@<code>{operator+(interval<T> lhs, typename interval<T>::value_type rhs)}が検索されます。
左オペランドから@<code>{interval<T>}は@<code>{interval<int>}と推論されます。
続いて、二段階名前検索によって依存名が解決され、@<code>{typename interval<T>::value_type}が@<code>{int}と導出されます。

最終的に、@<code>{1.0 (double)}は@<code>{int}に変換可能ですので、オーバーロード解決に成功します！

言語機能の悪用ってすごく楽しいですよね！

=== constexpr if

別に難しい機能じゃないが、二段階名前検索でハマる人が続出しているので解説することにしました（cpprefjp読めでもいいのですが…）。

問題のコードはつぎのようなものです:

//emlist[][cpp]{
template <class T>
void func() {
  if constexpr (!std::integral_v<T>) {
    // Tが整数型じゃないときのみ評価されてほしい
    // 実際は常に評価される
    static_assert(false);
  }
}
//}

constexpr ifは実行されないテンプレートの実体化を防ぐ (依存名の検証をしない) だけで、非依存名は検証されるのです。
この例のstatic_assertに渡す条件式はテンプレートパラメータに依存していないので、テンプレートの宣言時に検証されてエラーになります。

解決方法は簡単でstatic_assertに渡す条件式が依存名ならばテンプレートの宣言時に検証されず、テンプレート実体化まで評価を遅らせることができます。

//emlist[][cpp]{
template <class...> inline constexpr bool always_false_v = false;

template <class T>
void func() {
  if constexpr (std::integral_v<T>) {
    // ...
  }
  else {
    static_assert(always_false_v<T>);
  }
}
//}

@<code>{always_false_v<T>}は@<code>{T}に依存する式なので、依存名です。

また、つぎのようにラムダ式を使うことで評価を遅らせるという作戦も考えられます（ラムダ式のコンパイル時評価が許可されたC++17から可能）。

//emlist[][cpp]{
template <typename T>
void f(T) {
  if constexpr (std::is_same_v<T, int>) {
    static_assert([]{ return false; }());
  }
}
//}

== この章のまとめ

 * Forwarding Referenceを使う場合はオーバーロードに型制約を設ける
 * 新しい機能を使う前に、@<code>{cppreference.com}で一度調べる
 * constexpr ifで@<code>{static_assert}を使う場合は依存式を使う
