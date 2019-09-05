= 無駄なコードを減らす

コードは書けば書くほどバグが混入する。
無駄なコードの重複や複雑性を回避し、シンプルで美しいコードを維持することが設計の成功の鍵である。

== コピペ乱舞

無駄なコードを減らすための最初の方針は共通の処理を関数として共通化することである。
悲しいかな、世の中には同じコードこコピペによって増殖させてしまったコードがたくさんある。
そのようなコードは一部を変更すると他の変更が漏れ、容易にバグの原因となる。

そもそもの話、自分でコードを書かずにある程度信頼できるライブラリを使うことを考えるべきである。
この章では、C++の標準ライブラリが提供するカスタマイゼーションポイントを活用する方法をいくつか紹介します。

== カスタマイゼーションポイント

多くの言語では文字列化やオブジェクトの比較など、よくある操作をカスタマイズする方法は確立されています。

カスタマイゼーションポイントとは、特定の関数等の動作が呼び出している関数だと思ってください。
この関数を定義しておくことで、動作をカスタマイズすることができます。

C++で主に使われるカスタマイゼーションポイントについて解説します。

=== Stringifiying

JavaやC#では以下のようにすると上手くいきます。
しかし、C++ではプリミティブはメンバ関数を持てないので上手くいきません。

//emlist[Javaでありそうなコード][cpp]{
class Widget {
    Gadget a, b;
public:
    std::string toString() const {
        return a.toString() + " " + b.toString();
    }
};
//}

C++17までは以下のように@<code>{std::ostream}に出力する@<code>{operator<<}をカスタマイゼーションポイントに使うことが多いです。
@<code>{std::stringstream}に出力して@<code>{str()}で文字列を取り出すのです。
C++20から文字列フォーマットが入りますので期待しましょう。

//emlist[][cpp]{
class Widget {
    int a, b;
public:
    friend std::ostream&
    operator<<(std::ostream& os, const Widget& w) const {
        return os << a << " " << b;
    }
};
//}

=== 比較

比較のカスタマイゼーションポイントは2通りあります。

@<b>{方法1: 演算子のオーバーロード}

C++20から三方比較演算子 @<code>{operator<=>} を定義するとすべての比較演算子が自動実装されるのですが、C++17まではすべてを自分で実装するしかありません。
@<code>{==, !=, <, >, <=, >=}の6種類の演算子を適切にオーバーロードしましょう。


@<b>{方法２: CPO（カスタマイゼーションポイントオブジェクト）}

 * クラスの場合

std::mapやstd::setは、テンプレートパラメータとして比較演算を提供するクラスを指定できます。

第3テンプレート引数がカスタマイゼーションポイントオブジェクトになっています。
std::lessを特殊化するか、自分で作ったクラスを渡してあげることでstd::mapのキー比較をカスタマイズすることが可能です。

//emlist[std::mapのを降順にカスタマイズ][cpp]{
#include <map>
#include <functional>

int main() {
    std::map<std::string, int, std::greater<>> dict{};
}
//}

このように、カスタマイゼーションポイントをクラスから分離しています。
何も書かなければデフォルト実装が選択されます。
ユーザーが明示的にクラスを指定することで実装の詳細をユーザーが静的にインジェクションすることができるのです。
このカスタマイゼーションポイントの設計はポリシーと呼ばれています（次章で解説します）。

 * 関数の場合

比較を伴うアルゴリズムの関数はカスタマイゼーションポイントオブジェクトを受け取るようになっています。
std::sort,std::min,std::maxの第3引数などがそれです。

カスタマイゼーションポイントを関数から分離して引数にしています。
何も書かなければデフォルト実装が選択されます。
関数オブジェクトを渡すことでカスタマイズが可能であり、よくあるカスタマイゼーションポイントのもたせ方です。

//emlist[std::sortのカスタマイズ][cpp]{
#include <algorithm>
#include <vector>

int main() {
    std::vector<std::pair<int, int>> vec{ {1 ,2}, {2, 3}, {3, 4} };
    std::sort(vec.begin(), vec.end(), [](auto&& p1, auto&& p2){
        return p1.second < p2.second;
    });
}
//}

== この章のまとめ

 * コードをしっかり共通化する
 * そもそも自分でコードを書かずにライブラリを探す
 * 標準ライブラリのカスタマイゼーションポイントを利用する