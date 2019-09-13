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

== constexpr if

別に難しい機能じゃないが、ハマる人が続出しているので解説することにしました（cpprefjp読めでもいいのですが…）。

問題のコードは以下です:

//emlist[][cpp]{
template <class T>
void func() {
  if constexpr (std::integral_v<T>) {
    // ...
  }
  else {
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

また、つぎのようにラムダ式を使うことで評価を遅らせるという作戦も考えられます。

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
