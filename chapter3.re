= 賢い継承の使い方

継承は便利ですが、使い方を誤るとひどいコードを生み出しがちです。

//image[dot_schema_all_modules][複雑な継承の例][scale=0.5]{ 
//}

== 継承を使いこなせない

仕事でC++を書いるプロでも継承が使いこなせないプログラマはいます。

具体的には以下のようなコードを書いてしまいます:

 * final指定をしないクラスがvirtualなデストラクタを持たない
 * override指定しないvirtual関数を書く
 * 継承関係が複雑すぎる

final, overrideはC++11から入った文脈依存キーワードです。
基本的にすべての仮想関数はoverride指定すべきであり、指定しないのは単なるサボりです。
また、final指定されていない仮想クラスが仮想デストラクタを持たない場合、
スライス問題によりリソースがリークし、デバッグ困難なバグを生み出します。
このあたりのことができていないのは勉強不足としか言いようがありません。

//emlist[][cpp]{
namespace my_lib {
  struct base {
    virtual ~base() = default;
    virtual void method1() = 0;
    virtual void method2() = 0;
  };

  struct derived1: base {
    virtual ~derived1() = default;
    // まだoverrideするのでfinal指定しない
    virtual void method1() override { /*...*/ }
    // もうoverrideしないのでfinal指定でoverrideを禁止する
    virtual void method2() override final { /*...*/ }
  };

  struct derived2 final: derived1 {
    // final指定されているのでvirtualでなくてもよい
    ~derived2() = default;

    virtual void method1() override { /*...*/ }

    // derived1でfinal指定されているのでoverride不可能
    // virtual void method2() override final { /*...*/ }
  };
}
//}

== 神クラス降臨、継承乱舞

仕事をしていて基底クラスを継承した派生クラスが数十種類あるコードを見たことがあります。

これは俗に言う「神クラス」というアンチパターンです。
共通に使う関数を基底クラスで純粋仮想関数として設計するというやり方です。

ちょっと考えるとわかりますが、この方法では基底クラスと派生クラスは密結合であり、非常に変更がしづらいのです。
だいたいの場合、何かが間違っています。

「神クラス」の問題を解決する前に、コンポジションについて説明します。

== コンポジション

コンポジションを簡単に説明すると、クラスのある機能を外部から指定して埋め込む設計です。

メンバとしてなにかを持つというのがコンポジションのように語られているとこが多いです。
C++の場合、テンプレートの引数として関数を持つクラスを指定するPolicyクラスという方法が一般的に用いられています。
ポリシークラスが状態を必要とする場合をメンバ変数として持ち、
状態が必要ないのであればstatic関数を持つクラスとしてテンプレートに埋め込むのが普通です。

第2章で紹介したstd::mapでもこの手法が用いられています。

//emlist[][cpp]{
namespace std {
    template <class Key, class T,
        class Compare = less<Key>,
        class Allocator = allocator<pair<const Key, T> >
    > class map;
}
//}

CompareやAllocatorはメンバmapが変数としてオブジェクトを持っています（EBOのために継承になっているかもしれません）。
テンプレートにカスタマイズされた比較クラスやアロケータクラスを指定し、コンストラクタにインスタンスを渡します。
こうすることで、継承を使わなくても異なるクラスの動作を制御できました！

@<b>{型が違うから同じコンテナに格納したりできへんやんけ！}

そう、テンプレートのコンポジションでカスタマイズされたクラスは型が違うのです。
ちょっとアロケータが違うだけでもです。
アロケータが違うだけで別のコンテナに代入できないなんて使いにくいですよね？

== 局所的動的多相

型が違う、ならば動的多相なアロケータを使えばええやろ！

@<b>{クラスそのものを派生クラスにせずに動的多相なPolicyクラスを使う。}

コンテナの場合、例えば@<code>{std::vector<T, Allocator>}の@<code>{Allocator}を動的多相化すれば、問題は解決します。

@<code>{polymorphic_allocator}という動的多相なアロケータクラスがC++17の@<code>{<polymorphic_resource>}というヘッダに入りました。
@<code>{pmr}という名前空間にエイリアス宣言されたコンテナはアロケータのクラスに@<code>{polymorphic_allocator}を指定しています。

//emlist[][cpp]{
namespace std {
  // C++17から
  namespace pmr {
    template <class T>
      using vector = std::vector<T, polymorphic_allocator<T>>;
  }
}
//}

神クラスを継承するのではありません。
機能を分解し、ひとつひとつを動的多相化すべきなのです。

独立できる単位でポリシークラスに切り分けるべきです。
なんなら人間クラスに足クラスと腕クラスくらいの細かいポリシーがあっていいです。
義手と生体の腕だと機能が違うでしょうし。

== 静的インジェンクション

ついでに紹介いたします。
静的インジェクションは、自動的に実装がインジェクションされたりされなかったりするテクニックです。
@<code>{std::optional}に便利関数を生やした@<code>{Option}というクラスを作る場合を考えます。

@<code>{Option<Option<T>>}を@<code>{Option<T>}にフラット化するモナディック関数を作りたいです。
@<code>{Option<T>}を@<code>{T}に持つ@<code>{Option}だけにこの関数を実装するにはC++20のrequires式を...
使いたいですが！

C++17まではつぎのように継承により実装をインジェクションします。

//emlist[][cpp]{
namespace option { // forward declaration
template <class T> class Option;
} // ! namespace option

namespace option::detail {
template <class> struct flatten_injector { void flatten() = delete; };

template <class T>
struct flatten_injector<Option<Option<T>>> {
    auto flatten() const& {
        return static_cast<const Option<Option<T>>*>(this)->storage.has_value()
            ? static_cast<const Option<Option<T>>*>(this)->storage.value()
            : Option<T>{};
    }
};
} // ! namespace option::detail

#include <optional>
namespace option {
template <class T>
class Option: public detail::flatten_injector<Option<T>> {
    template <class> friend class detail::flatten_injector;
    std::optional<T> storage;
  public:
    Option() = default;
    Option(T x): storage(x) {}
};
} // ! namespace option

int main(){
    option::Option<option::Option<int>> opt = option::Option(1);
    opt.flatten();
}
//}

== この章のまとめ

 * 機能を小さく分解する
 * 継承を局所化してポリシーにする
 * 最小の型制約を意識する
